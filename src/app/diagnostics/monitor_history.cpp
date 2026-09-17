// TODO: NEEDS REVIEW
#include "monitor_history.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <vector>

namespace firelight::diagnostics {

namespace {

/** A mean and a spread over whatever was fed in */
struct Running {
  double sum = 0.0;
  double sumOfSquares = 0.0;
  int64_t count = 0;

  void add(const double value) {
    sum += value;
    sumOfSquares += value * value;
    count++;
  }

  [[nodiscard]] double mean() const { return count > 0 ? sum / static_cast<double>(count) : 0.0; }

  [[nodiscard]] double deviationPercent() const {
    if (count < 2 || mean() <= 0.0) {
      return 0.0;
    }

    return spread() / mean() * 100.0;
  }

  [[nodiscard]] double spread() const {
    if (count < 2) {
      return 0.0;
    }

    const auto variance = std::max(sumOfSquares / static_cast<double>(count) - mean() * mean(), 0.0);
    return std::sqrt(variance);
  }
};

// TODO
/** A mean and a spread of offsets into a repeating period, taken around the period rather than along a line */
struct RunningPhase {
  double periodNs = 0.0;
  std::vector<double> offsetsNs;

  // TODO
  /** Adds an offset, folded into the period */
  void add(const double offsetNs) { offsetsNs.push_back(std::fmod(offsetNs, periodNs)); }

  // TODO
  /** @return Where in the period the offsets gather, from 0 up to the period */
  [[nodiscard]] double mean() const {
    if (offsetsNs.empty() || periodNs <= 0.0) {
      return 0.0;
    }

    auto sumOfSines = 0.0;
    auto sumOfCosines = 0.0;

    for (const auto offsetNs : offsetsNs) {
      const auto angle = offsetNs / periodNs * 2.0 * std::numbers::pi;
      sumOfSines += std::sin(angle);
      sumOfCosines += std::cos(angle);
    }

    const auto meanNs = std::atan2(sumOfSines, sumOfCosines) / (2.0 * std::numbers::pi) * periodNs;
    return meanNs < 0.0 ? meanNs + periodNs : meanNs;
  }

  // TODO
  /** @return How far the offsets stray from mean(), each measured the short way round */
  [[nodiscard]] double spread() const {
    if (offsetsNs.size() < 2 || periodNs <= 0.0) {
      return 0.0;
    }

    const auto centreNs = mean();
    Running around;

    for (const auto offsetNs : offsetsNs) {
      around.add(std::remainder(offsetNs - centreNs, periodNs));
    }

    return around.spread();
  }
};

constexpr double NS_PER_MS = 1e6;
constexpr double NS_PER_S = 1e9;

} // namespace

MonitorHistory::MonitorHistory(monitoring::Monitor &monitor) : m_monitor(monitor) {}

void MonitorHistory::update() {
  refreshKinds();
  m_monitor.collect(m_cursor, m_scratch);
  m_updatedNs = monitoring::Monitor::now();
  accumulate(m_scratch);

  // TODO
  // The batch is time-ordered and so is the history; a merge keeps it that way for less than a sort
  const auto spansBefore = m_spans.size();
  m_spans.insert(m_spans.end(), m_scratch.spans.begin(), m_scratch.spans.end());
  std::inplace_merge(m_spans.begin(), m_spans.begin() + static_cast<std::ptrdiff_t>(spansBefore), m_spans.end(),
                     [](const auto &a, const auto &b) { return a.startNs < b.startNs; });
  const auto samplesBefore = m_samples.size();
  m_samples.insert(m_samples.end(), m_scratch.samples.begin(), m_scratch.samples.end());
  std::inplace_merge(m_samples.begin(), m_samples.begin() + static_cast<std::ptrdiff_t>(samplesBefore), m_samples.end(),
                     [](const auto &a, const auto &b) { return a.atNs < b.atNs; });
  prune();
}

void MonitorHistory::reset() {
  m_spans.clear();
  m_samples.clear();
  m_framesRun = 0;
  m_pacedFrames = 0;
  m_pacedElapsedNs = 0;
  m_lastFrameStartNs = 0;
  m_framesLost = 0;
  m_framesShown = 0;
  m_framesRepeated = 0;
  m_framesNoPicture = 0;
  m_blanks = 0;
  m_presents = 0;
  m_passes = 0;
  m_samplesDelivered = 0;
  m_wakeOvershootPeakMs = 0.0;
  m_hasSeenBlank = false;
  m_presentsSinceBlank = 0;
  m_framesSinceBlank = 0;
  m_blanksWithoutPresent = 0;
  m_blanksWithTwoPresents = 0;
  m_blanksWithoutFrame = 0;
  m_blanksWithTwoFrames = 0;
  m_slips = 0;
  m_timeDrops = 0;
  m_lappedRecords = 0;
  m_recentSlipsNs.clear();
  m_unmatchedRepeatsNs.clear();
  m_repeatsNearSlip = 0;
}

monitoring::KindId MonitorHistory::getKind(const std::string_view name) const {
  const auto found = m_kinds.find(std::string(name));
  return found != m_kinds.end() ? found->second : 0;
}

const std::vector<monitoring::SpanRecord> &MonitorHistory::getSpans() const { return m_spans; }

const std::vector<monitoring::SampleRecord> &MonitorHistory::getSamples() const { return m_samples; }

int64_t MonitorHistory::getUpdatedNs() const { return m_updatedNs; }

MonitorFigures MonitorHistory::getFigures() const {
  MonitorFigures figures;
  figures.framesRun = m_framesRun;
  figures.framesLost = m_framesLost;
  figures.framesShown = m_framesShown;
  figures.framesNotShown = std::max<int64_t>(m_framesRun - m_framesShown, 0);
  figures.framesRepeated = m_framesRepeated;
  figures.repeatsNearSlip = m_repeatsNearSlip;
  figures.framesNoPicture = m_framesNoPicture;
  figures.blanks = m_blanks;
  figures.presents = m_presents;
  figures.passes = m_passes;
  figures.samplesDelivered = m_samplesDelivered;
  figures.wakeOvershootPeakMs = m_wakeOvershootPeakMs;
  figures.blanksWithoutPresent = m_blanksWithoutPresent;
  figures.blanksWithTwoPresents = m_blanksWithTwoPresents;
  figures.blanksWithoutFrame = m_blanksWithoutFrame;
  figures.blanksWithTwoFrames = m_blanksWithTwoFrames;
  figures.slips = m_slips;
  figures.timeDrops = m_timeDrops;
  figures.lappedRecords = m_lappedRecords;
  figures.droppedRecords = m_scratch.droppedRecords;

  const auto windowStartNs = m_updatedNs - static_cast<int64_t>(FIGURE_WINDOW_SECONDS * NS_PER_S);
  const auto runFrame = getKind("run_frame");
  const auto present = getKind("present");

  // TODO
  // Frame time is frame start to frame start; submit time is present to present
  Running frameGaps;
  Running submitGaps;
  int64_t lastFrameStartNs = 0;
  int64_t lastPresentNs = 0;

  for (const auto &span : m_spans) {
    if (span.startNs < windowStartNs) {
      continue;
    }

    if (span.kind == present) {
      if (lastPresentNs > 0 && span.startNs - lastPresentNs <= MAX_FRAME_GAP_NS) {
        submitGaps.add(static_cast<double>(span.startNs - lastPresentNs));
      }

      lastPresentNs = span.startNs;
    } else if (span.kind == runFrame) {
      if (lastFrameStartNs > 0 && span.startNs - lastFrameStartNs <= MAX_FRAME_GAP_NS) {
        frameGaps.add(static_cast<double>(span.startNs - lastFrameStartNs));
      }

      lastFrameStartNs = span.startNs;
    }
  }

  figures.frameTimeMs = frameGaps.mean() / NS_PER_MS;
  figures.frameTimeDeviationPercent = frameGaps.deviationPercent();
  figures.submitTimeMs = submitGaps.mean() / NS_PER_MS;
  figures.submitDeviationPercent = submitGaps.deviationPercent();

  // TODO
  // Where a submit, and the end of the wait before it, fall after the vertical blank last seen
  const auto vblank = getKind("vblank");
  const auto swapchainWait = getKind("swapchain_wait");
  std::vector<int64_t> vblanksNs;

  for (const auto &span : m_spans) {
    if (span.kind == vblank && span.startNs >= windowStartNs) {
      vblanksNs.push_back(span.startNs);
    }
  }

  const auto lastVblankBefore = [&vblanksNs](const int64_t atNs) -> int64_t {
    const auto after = std::upper_bound(vblanksNs.begin(), vblanksNs.end(), atNs);
    return after == vblanksNs.begin() ? 0 : *(after - 1);
  };

  // TODO
  // The refresh period, as the middle of the gaps between the blanks seen
  std::vector<int64_t> blankGapsNs;

  for (size_t index = 1; index < vblanksNs.size(); ++index) {
    blankGapsNs.push_back(vblanksNs[index] - vblanksNs[index - 1]);
  }

  auto refreshNs = 0.0;

  if (!blankGapsNs.empty()) {
    const auto middle = blankGapsNs.begin() + static_cast<std::ptrdiff_t>(blankGapsNs.size() / 2);
    std::nth_element(blankGapsNs.begin(), middle, blankGapsNs.end());
    refreshNs = static_cast<double>(*middle);
  }

  const auto frameGrab = getKind("frame_grab");
  Running vblankToSubmit;
  Running vblankToWaitEnd;
  RunningPhase vblankToFrameEnd{.periodNs = refreshNs, .offsetsNs = {}};
  RunningPhase vblankToFrameGrab{.periodNs = refreshNs, .offsetsNs = {}};
  auto presentsInWindow = 0;
  auto framesInWindow = 0;

  for (const auto &span : m_spans) {
    if (span.startNs < windowStartNs) {
      continue;
    }

    if (span.kind == present) {
      presentsInWindow++;
      const auto vblankNs = lastVblankBefore(span.startNs);

      if (vblankNs > 0 && span.startNs - vblankNs <= MAX_FRAME_GAP_NS) {
        vblankToSubmit.add(static_cast<double>(span.startNs - vblankNs));
      }
    } else if (span.kind == swapchainWait) {
      const auto vblankNs = lastVblankBefore(span.endNs);

      if (vblankNs > 0 && span.endNs - vblankNs <= MAX_FRAME_GAP_NS) {
        vblankToWaitEnd.add(static_cast<double>(span.endNs - vblankNs));
      }
    } else if (span.kind == runFrame) {
      framesInWindow++;
      const auto vblankNs = lastVblankBefore(span.endNs);

      if (vblankNs > 0 && refreshNs > 0.0 && span.endNs - vblankNs <= MAX_FRAME_GAP_NS) {
        vblankToFrameEnd.add(static_cast<double>(span.endNs - vblankNs));
      }
    } else if (span.kind == frameGrab && refreshNs > 0.0) {
      const auto vblankNs = lastVblankBefore(span.startNs);

      if (vblankNs > 0 && span.startNs - vblankNs <= MAX_FRAME_GAP_NS) {
        vblankToFrameGrab.add(static_cast<double>(span.startNs - vblankNs));
      }
    }
  }

  if (!vblanksNs.empty()) {
    figures.presentsPerBlank = static_cast<double>(presentsInWindow) / static_cast<double>(vblanksNs.size());
    figures.framesPerBlank = static_cast<double>(framesInWindow) / static_cast<double>(vblanksNs.size());
  }

  figures.vblankToSubmitMs = vblankToSubmit.mean() / NS_PER_MS;
  figures.vblankToWaitEndMs = vblankToWaitEnd.mean() / NS_PER_MS;
  figures.vblankToFrameEndMs = vblankToFrameEnd.mean() / NS_PER_MS;
  figures.frameEndSpreadMs = vblankToFrameEnd.spread() / NS_PER_MS;
  figures.vblankToFrameGrabMs = vblankToFrameGrab.mean() / NS_PER_MS;
  figures.frameGrabSpreadMs = vblankToFrameGrab.spread() / NS_PER_MS;

  if (m_pacedElapsedNs > 0 && m_pacedFrames > 0) {
    figures.frameRate = static_cast<double>(m_pacedFrames) * NS_PER_S / static_cast<double>(m_pacedElapsedNs);
  }

  const auto spinMargin = getKind("spin_margin");
  const auto wakeOvershoot = getKind("wake_overshoot");
  const auto audioBuffer = getKind("audio_buffer");
  const auto audioCorrection = getKind("audio_correction");
  const auto phaseLocked = getKind("phase_locked");
  const auto slipDebt = getKind("slip_debt");
  const auto refreshPeriod = getKind("refresh_period");
  Running overshoot;
  Running occupancy;
  auto nearUnderrun = 0;
  auto nearBlocking = 0;

  for (const auto &sample : m_samples) {
    if (sample.kind == spinMargin) {
      figures.spinMarginMs = sample.value;
    } else if (sample.kind == audioCorrection) {
      figures.correctionPercent = sample.value * 100.0;
    } else if (sample.kind == phaseLocked) {
      figures.phaseLocked = sample.value > 0.5;
    } else if (sample.kind == refreshPeriod) {
      figures.refreshPeriodMs = sample.value;
    } else if (sample.kind == slipDebt) {
      figures.slipDebtMs = sample.value;
    }

    if (sample.atNs < windowStartNs) {
      continue;
    }

    if (sample.kind == wakeOvershoot) {
      overshoot.add(sample.value);
    } else if (sample.kind == audioBuffer) {
      occupancy.add(sample.value);
      nearUnderrun += sample.value <= UNDERRUN_LEVEL ? 1 : 0;
      nearBlocking += sample.value >= BLOCKING_LEVEL ? 1 : 0;
    }
  }

  figures.wakeOvershootMeanMs = overshoot.mean();
  figures.bufferSaturationPercent = occupancy.mean() * 100.0;
  figures.bufferDeviationPercent = occupancy.deviationPercent();

  if (occupancy.count > 0) {
    figures.closeToUnderrunPercent = static_cast<double>(nearUnderrun) / static_cast<double>(occupancy.count) * 100.0;
    figures.closeToBlockingPercent = static_cast<double>(nearBlocking) / static_cast<double>(occupancy.count) * 100.0;
  }

  return figures;
}

void MonitorHistory::refreshKinds() {
  const auto kinds = m_monitor.getKinds();

  if (kinds.size() == m_kindCount) {
    return;
  }

  m_kinds.clear();

  for (const auto &kind : kinds) {
    m_kinds[kind.name] = kind.id;
  }

  m_kindCount = kinds.size();
}

void MonitorHistory::accumulate(const monitoring::Snapshot &snapshot) {
  const auto runFrame = getKind("run_frame");
  const auto frameDropped = getKind("frame_dropped");
  const auto frameShown = getKind("frame_shown");
  const auto frameRepeated = getKind("frame_repeated");
  const auto frameNoPicture = getKind("frame_no_picture");
  const auto slip = getKind("slip");
  const auto timeDropped = getKind("time_dropped");
  const auto vblank = getKind("vblank");
  const auto present = getKind("present");
  const auto renderPass = getKind("render_pass");
  const auto wakeOvershoot = getKind("wake_overshoot");
  const auto audioSamples = getKind("audio_samples");
  m_lappedRecords += snapshot.lappedRecords;

  for (const auto &span : snapshot.spans) {
    // TODO
    // Frames before the first one shown ran behind the launch, and never had a pass to reach
    if (span.kind == runFrame && m_framesShown > 0) {
      m_framesRun++;

      // TODO
      // Counted since the start rather than over a window, because a window holds a whole number of
      // frames against a length that is not one, and a doubled frame in it reads as half a percent
      if (m_lastFrameStartNs > 0 && span.startNs - m_lastFrameStartNs <= MAX_FRAME_GAP_NS) {
        m_pacedElapsedNs += span.startNs - m_lastFrameStartNs;
        m_pacedFrames++;
      }

      m_lastFrameStartNs = span.startNs;
    } else if (span.kind == frameDropped) {
      m_framesLost++;
    } else if (span.kind == frameShown) {
      m_framesShown++;
    } else if (span.kind == frameRepeated) {
      m_framesRepeated++;
      noteRepeat(span.startNs);
    } else if (span.kind == frameNoPicture) {
      m_framesNoPicture++;
    } else if (span.kind == slip) {
      m_slips++;
      noteSlip(span.startNs);
    } else if (span.kind == timeDropped) {
      m_timeDrops++;
    } else if (span.kind == vblank && m_framesShown > 0) {
      m_blanks++;
    } else if (span.kind == present && m_framesShown > 0) {
      m_presents++;
    } else if (span.kind == renderPass && m_framesShown > 0) {
      m_passes++;
    }
  }

  // TODO
  // Each blank interval is filled by the presents and frames between one blank and the next, in
  // time order, and is counted once the blank that closes it has been seen
  {
    std::vector<std::pair<int64_t, int>> events;

    for (const auto &span : snapshot.spans) {
      if (span.kind == vblank) {
        events.emplace_back(span.startNs, 0);
      } else if (span.kind == present) {
        events.emplace_back(span.startNs, 1);
      } else if (span.kind == runFrame) {
        events.emplace_back(span.startNs, 2);
      }
    }

    std::sort(events.begin(), events.end());

    for (const auto &[atNs, what] : events) {
      if (what == 1) {
        m_presentsSinceBlank++;
        continue;
      }

      if (what == 2) {
        m_framesSinceBlank++;
        continue;
      }

      if (m_hasSeenBlank && m_framesShown > 0) {
        m_blanksWithoutPresent += m_presentsSinceBlank == 0 ? 1 : 0;
        m_blanksWithTwoPresents += m_presentsSinceBlank >= 2 ? 1 : 0;
        m_blanksWithoutFrame += m_framesSinceBlank == 0 ? 1 : 0;
        m_blanksWithTwoFrames += m_framesSinceBlank >= 2 ? 1 : 0;
      }

      m_hasSeenBlank = true;
      m_presentsSinceBlank = 0;
      m_framesSinceBlank = 0;
    }
  }

  for (const auto &sample : snapshot.samples) {
    if (sample.kind == audioSamples) {
      m_samplesDelivered += static_cast<int64_t>(sample.value);
    } else if (sample.kind == wakeOvershoot) {
      m_wakeOvershootPeakMs = std::max(m_wakeOvershootPeakMs, sample.value);
    }
  }
}

void MonitorHistory::prune() {
  const auto keepFromNs = m_updatedNs - static_cast<int64_t>(KEEP_SECONDS * NS_PER_S);
  const auto spanCut = std::find_if(m_spans.begin(), m_spans.end(),
                                    [keepFromNs](const auto &record) { return record.endNs >= keepFromNs; });
  m_spans.erase(m_spans.begin(), spanCut);
  const auto sampleCut = std::find_if(m_samples.begin(), m_samples.end(),
                                      [keepFromNs](const auto &record) { return record.atNs >= keepFromNs; });
  m_samples.erase(m_samples.begin(), sampleCut);
}

void MonitorHistory::noteRepeat(const int64_t atNs) {
  forgetBefore(atNs);
  const auto isNearSlip = std::any_of(m_recentSlipsNs.begin(), m_recentSlipsNs.end(),
                                      [atNs](const int64_t slipNs) { return std::abs(atNs - slipNs) <= NEAR_SLIP_NS; });

  if (isNearSlip) {
    m_repeatsNearSlip++;
    return;
  }

  m_unmatchedRepeatsNs.push_back(atNs);
}

void MonitorHistory::noteSlip(const int64_t atNs) {
  forgetBefore(atNs);
  const auto matched = std::erase_if(
      m_unmatchedRepeatsNs, [atNs](const int64_t repeatNs) { return std::abs(atNs - repeatNs) <= NEAR_SLIP_NS; });
  m_repeatsNearSlip += static_cast<int64_t>(matched);
  m_recentSlipsNs.push_back(atNs);
}

void MonitorHistory::forgetBefore(const int64_t atNs) {
  const auto oldestNs = atNs - 2 * NEAR_SLIP_NS;
  std::erase_if(m_recentSlipsNs, [oldestNs](const int64_t slipNs) { return slipNs < oldestNs; });
  std::erase_if(m_unmatchedRepeatsNs, [oldestNs](const int64_t repeatNs) { return repeatNs < oldestNs; });
}

} // namespace firelight::diagnostics
