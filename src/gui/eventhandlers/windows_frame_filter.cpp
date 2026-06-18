#include "windows_frame_filter.hpp"
#include <QWindow>
#include <dwmapi.h>

WindowsFrameFilter::WindowsFrameFilter(QObject *parent) : QObject(parent) {
}

void WindowsFrameFilter::setWindow(QWindow *window) {
    m_hwnd = reinterpret_cast<HWND>(window->winId());
    m_dpr = window->devicePixelRatio();

    // Keep DWM shadow when using WM_NCCALCSIZE trick
    MARGINS m{1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(m_hwnd, &m);

    SetWindowPos(m_hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

bool WindowsFrameFilter::nativeEventFilter(const QByteArray &eventType,
                                           void *message, qintptr *result) {
    if (eventType != "windows_generic_MSG") return false;
    MSG *msg = static_cast<MSG *>(message);
    if (!m_hwnd || msg->hwnd != m_hwnd) return false;

    switch (msg->message) {
        case WM_NCCALCSIZE: {
            if (msg->wParam == FALSE) return false;
            // When maximized, Windows adds padding to push content off-screen edges.
            // Read it back so our client area doesn't get clipped.
            if (IsZoomed(m_hwnd)) {
                auto *p = reinterpret_cast<NCCALCSIZE_PARAMS *>(msg->lParam);
                int pad = GetSystemMetrics(SM_CXSIZEFRAME)
                          + GetSystemMetrics(SM_CXPADDEDBORDER);
                p->rgrc[0].left   += pad;
                p->rgrc[0].top    += pad;
                p->rgrc[0].right  -= pad;
                p->rgrc[0].bottom -= pad;
            }
            *result = 0;
            return true;
        }

        case WM_NCHITTEST: {
            POINT pt{GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};
            RECT wr;
            GetWindowRect(m_hwnd, &wr);

            // Physical pixels relative to window origin
            int px = pt.x - wr.left;
            int py = pt.y - wr.top;
            int pw = wr.right - wr.left;
            int ph = wr.bottom - wr.top;

            // Resize borders (skip when maximized or fullscreen)
            if (!IsZoomed(m_hwnd) && !IsIconic(m_hwnd)) {
                int b = m_border;
                bool L = px < b, R = px >= pw - b;
                bool T = py < b, B = py >= ph - b;
                if (T && L) {
                    *result = HTTOPLEFT;
                    return true;
                }
                if (T && R) {
                    *result = HTTOPRIGHT;
                    return true;
                }
                if (B && L) {
                    *result = HTBOTTOMLEFT;
                    return true;
                }
                if (B && R) {
                    *result = HTBOTTOMRIGHT;
                    return true;
                }
                if (L) {
                    *result = HTLEFT;
                    return true;
                }
                if (R) {
                    *result = HTRIGHT;
                    return true;
                }
                if (T) {
                    *result = HTTOP;
                    return true;
                }
                if (B) {
                    *result = HTBOTTOM;
                    return true;
                }
            }

            // Convert to logical pixels for QML rect comparison
            int lx = static_cast<int>(px / m_dpr);
            int ly = static_cast<int>(py / m_dpr);
            QPoint lp(lx, ly);

            *result = HTCLIENT;
            return true;
        }

        case WM_NCACTIVATE:
            // Suppress the default non-client area repaint (we draw our own)
            *result = TRUE;
            return true;
    }

    return false;
}
