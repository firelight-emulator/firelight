#pragma once
#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QWindow>
#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>  // GET_X_LPARAM, GET_Y_LPARAM
#endif

class WindowsFrameFilter : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT

public:
    explicit WindowsFrameFilter(QObject *parent = nullptr);

    Q_INVOKABLE void setWindow(QWindow *window);

    // Outer frame position in logical (QML) pixels via GetWindowRect,
    // bypassing Qt's frame-margin adjustment.
    Q_INVOKABLE int nativeX() const;
    Q_INVOKABLE int nativeY() const;

    // Move the outer frame to (logicalX, logicalY) via SetWindowPos,
    // bypassing Qt's frame-margin subtraction.
    Q_INVOKABLE void setNativePosition(int logicalX, int logicalY);

    bool nativeEventFilter(const QByteArray &eventType,
                           void *message, qintptr *result) override;

private:
#ifdef _WIN32
    HWND m_hwnd = nullptr;
#else
    QWindow *m_window = nullptr;
#endif
    double m_dpr = 1.0;
    int m_border = 8; // resize border width in physical pixels
};
