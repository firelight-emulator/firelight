#pragma once
#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QWindow>
#include <windows.h>
#include <windowsx.h>  // GET_X_LPARAM, GET_Y_LPARAM

class WindowsFrameFilter : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT

public:
    explicit WindowsFrameFilter(QObject *parent = nullptr);

    // Called from QML to register hit zones (in logical pixels)
    Q_INVOKABLE void setWindow(QWindow *window);


    bool nativeEventFilter(const QByteArray &eventType,
                           void *message, qintptr *result) override;

private:
    HWND m_hwnd = nullptr;
    double m_dpr = 1.0;
    int m_border = 8; // resize border width in physical pixels
};
