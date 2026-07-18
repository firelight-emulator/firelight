#include "windows_frame_filter.hpp"
#include <QWindow>
#ifdef _WIN32
#include <dwmapi.h>
#endif

WindowsFrameFilter::WindowsFrameFilter(QObject *parent) : QObject(parent) {
}

#ifdef _WIN32
void WindowsFrameFilter::setWindow(QWindow *window) {
    m_hwnd = reinterpret_cast<HWND>(window->winId());
    m_dpr = window->devicePixelRatio();

    // Qt's logical size is the true desired content size. GetWindowRect would
    // return an inflated outer rect (Qt added title bar + borders via
    // AdjustWindowRect), so we convert the logical size directly instead
    int physW = static_cast<int>(window->width()  * m_dpr);
    int physH = static_cast<int>(window->height() * m_dpr);

    MARGINS m{1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(m_hwnd, &m);

    // Activate our WM_NCCALCSIZE handler (client area = full window rect)
    SetWindowPos(m_hwnd, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    // Nudge forces WM_SIZE so Qt re-queries GetClientRect. Because
    // WM_NCCALCSIZE now returns client=window, Qt sees physW x physH and
    // resets contentItem to (0,0) with the correct dimensions
    SetWindowPos(m_hwnd, nullptr, 0, 0, physW, physH + 1,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(m_hwnd, nullptr, 0, 0, physW, physH,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

int WindowsFrameFilter::nativeX() const {
    if (!m_hwnd) return 0;
    RECT wr;
    GetWindowRect(m_hwnd, &wr);
    return static_cast<int>(wr.left / m_dpr);
}

int WindowsFrameFilter::nativeY() const {
    if (!m_hwnd) return 0;
    RECT wr;
    GetWindowRect(m_hwnd, &wr);
    return static_cast<int>(wr.top / m_dpr);
}

void WindowsFrameFilter::setNativePosition(int logicalX, int logicalY) {
    if (!m_hwnd) return;
    int px = static_cast<int>(logicalX * m_dpr);
    int py = static_cast<int>(logicalY * m_dpr);
    SetWindowPos(m_hwnd, nullptr, px, py, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

bool WindowsFrameFilter::nativeEventFilter(const QByteArray &eventType,
                                           void *message, qintptr *result) {
    if (eventType != "windows_generic_MSG") return false;
    MSG *msg = static_cast<MSG *>(message);
    if (!m_hwnd || msg->hwnd != m_hwnd) return false;

    switch (msg->message) {
        case WM_NCCALCSIZE: {
            if (msg->wParam == FALSE) return false;
            // When maximized, Windows adds padding to push content off-screen edges
            // Read it back so our client area doesn't get clipped
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

#else // !_WIN32

// Non-Windows platforms use the compositor's native title bar, so there's no
// custom non-client hit-testing to do. We keep the class (and the WindowFrame
// QML API) alive with Qt-based equivalents so MainWindow.qml still works: the
// window position save/restore is driven through QWindow instead of Win32.

void WindowsFrameFilter::setWindow(QWindow *window) { m_window = window; }

int WindowsFrameFilter::nativeX() const { return m_window ? m_window->x() : 0; }

int WindowsFrameFilter::nativeY() const { return m_window ? m_window->y() : 0; }

void WindowsFrameFilter::setNativePosition(int logicalX, int logicalY) {
    if (m_window)
        m_window->setPosition(logicalX, logicalY);
}

bool WindowsFrameFilter::nativeEventFilter(const QByteArray &, void *, qintptr *) {
    return false;
}

#endif // _WIN32
