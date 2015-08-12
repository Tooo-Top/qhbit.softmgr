#include "MainWnd.h"
#include <qt_windows.h>


MainWnd::MainWnd(QWidget *parent) : QWebView(parent)
{
    setMouseTracking(true);
}

bool MainWnd::nativeEvent(const QByteArray & eventType, void * message, long * result) {
     MSG* msg = reinterpret_cast<MSG*>(message);
     if (msg->message == WM_LBUTTONDOWN) {
         ::SendMessage(HWND(this->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
     }

	 return QWebView::nativeEvent(eventType, message, result);
}

