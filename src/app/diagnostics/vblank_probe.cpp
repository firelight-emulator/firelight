// TODO: NEEDS REVIEW
#include "vblank_probe.hpp"

#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <windows.h>
#endif

namespace firelight::diagnostics {

#ifdef _WIN32
namespace {

// TODO
// The kernel-mode thunk ABI from d3dkmthk.h, which the toolchain does not ship
using KmtHandle = UINT;

struct OpenAdapterFromHdc {
  HDC hDc = nullptr;
  KmtHandle hAdapter = 0;
  LUID AdapterLuid{};
  UINT VidPnSourceId = 0;
};

struct CloseAdapter {
  KmtHandle hAdapter = 0;
};

struct WaitForVerticalBlankEvent {
  KmtHandle hAdapter = 0;
  KmtHandle hDevice = 0;
  UINT VidPnSourceId = 0;
};

using OpenAdapterFn = LONG(WINAPI *)(OpenAdapterFromHdc *);
using CloseAdapterFn = LONG(WINAPI *)(const CloseAdapter *);
using WaitForVerticalBlankFn = LONG(WINAPI *)(const WaitForVerticalBlankEvent *);

} // namespace
#endif

VblankProbe::~VblankProbe() { stop(); }

void VblankProbe::follow(const std::string &displayName) {
  if (m_running.load(std::memory_order_acquire) && displayName == m_displayName) {
    return;
  }

  stop();
  m_displayName = displayName;
  m_running.store(true, std::memory_order_release);
  m_thread = std::thread([this, displayName] { run(displayName); });
}

void VblankProbe::stop() {
  m_running.store(false, std::memory_order_release);

  if (m_thread.joinable()) {
    m_thread.join();
  }
}

void VblankProbe::run(const std::string &displayName) const {
#ifdef _WIN32
  const auto gdi = LoadLibraryW(L"gdi32.dll");

  if (gdi == nullptr) {
    return;
  }

  const auto openAdapter = reinterpret_cast<OpenAdapterFn>(GetProcAddress(gdi, "D3DKMTOpenAdapterFromHdc"));
  const auto closeAdapter = reinterpret_cast<CloseAdapterFn>(GetProcAddress(gdi, "D3DKMTCloseAdapter"));
  const auto waitForVerticalBlank =
      reinterpret_cast<WaitForVerticalBlankFn>(GetProcAddress(gdi, "D3DKMTWaitForVerticalBlankEvent"));

  if (openAdapter == nullptr || closeAdapter == nullptr || waitForVerticalBlank == nullptr) {
    spdlog::warn("Vblank probe: the display driver thunks are not available");
    return;
  }

  const std::wstring wideName(displayName.begin(), displayName.end());
  const auto hdc = CreateDCW(L"DISPLAY", wideName.c_str(), nullptr, nullptr);

  if (hdc == nullptr) {
    spdlog::warn("Vblank probe: no device context for display {}", displayName);
    return;
  }

  OpenAdapterFromHdc open;
  open.hDc = hdc;
  const auto opened = openAdapter(&open);
  DeleteDC(hdc);

  if (opened != 0) {
    spdlog::warn("Vblank probe: could not open the adapter for display {} (status {})", displayName, opened);
    return;
  }

  spdlog::info("Vblank probe: following display {}", displayName);
  WaitForVerticalBlankEvent wait;
  wait.hAdapter = open.hAdapter;
  wait.VidPnSourceId = open.VidPnSourceId;

  while (m_running.load(std::memory_order_acquire)) {
    if (waitForVerticalBlank(&wait) != 0) {
      Sleep(1);
      continue;
    }

    m_vblankMarker.mark();
  }

  CloseAdapter close;
  close.hAdapter = open.hAdapter;
  closeAdapter(&close);
#else
  (void)displayName;
#endif
}

} // namespace firelight::diagnostics
