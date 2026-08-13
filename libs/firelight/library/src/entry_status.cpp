#include <firelight/library/entry_status.hpp>

#include <algorithm>

namespace firelight::library {

bool EntryStatus::isPlayable() const {
  return std::ranges::none_of(problems, [](const EntryProblem problem) { return blocksLaunch(problem); });
}

bool blocksLaunch(const EntryProblem problem) {
  switch (problem) {
  case EntryProblem::PlatformNotSupported:
  case EntryProblem::CoreNotInstalled:
  case EntryProblem::BiosMissing:
  case EntryProblem::ContentUnavailable:
  case EntryProblem::FilesMissing:
  case EntryProblem::ContentInArchive:
    return true;
  case EntryProblem::DiscsMissing:
    return false;
  }

  return true;
}

int severity(const EntryProblem problem) {
  switch (problem) {
  case EntryProblem::PlatformNotSupported:
    return 0;
  case EntryProblem::CoreNotInstalled:
    return 1;
  case EntryProblem::BiosMissing:
    return 2;
  case EntryProblem::ContentUnavailable:
    return 3;
  case EntryProblem::FilesMissing:
    return 4;
  case EntryProblem::ContentInArchive:
    return 5;
  case EntryProblem::DiscsMissing:
    return 6;
  }

  return 7;
}

EntryStatus evaluateEntryStatus(const EntryStatusFacts &facts) {
  EntryStatus status;

  if (!facts.isCoreRegistered) {
    status.problems.push_back(EntryProblem::PlatformNotSupported);
  } else if (!facts.isCoreInstalled) {
    // Only when a core exists to install. Otherwise this repeats what the platform already said
    status.problems.push_back(EntryProblem::CoreNotInstalled);
  } else if (!facts.hasRequiredBios) {
    // TODO
    // Only once the core is there. Naming the BIOS a core needs before that core exists sends
    // somebody hunting for a file they cannot use yet
    status.problems.push_back(EntryProblem::BiosMissing);
  }

  // TODO
  // A folder that cannot be read says nothing about whether the files under it are gone, so
  // reporting both would tell somebody their games are lost when the drive is merely unplugged
  if (!facts.isContentReachable) {
    status.problems.push_back(EntryProblem::ContentUnavailable);
  } else if (!facts.hasWayIn) {
    status.problems.push_back(EntryProblem::FilesMissing);
  }

  if (facts.isDiscInArchive) {
    status.problems.push_back(EntryProblem::ContentInArchive);
  }

  if (facts.expectedDiscCount > 0 && facts.presentDiscCount != facts.expectedDiscCount) {
    status.problems.push_back(EntryProblem::DiscsMissing);
  }

  std::ranges::sort(status.problems,
                    [](const EntryProblem left, const EntryProblem right) { return severity(left) < severity(right); });

  return status;
}

} // namespace firelight::library
