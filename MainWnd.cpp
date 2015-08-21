#include "MainWnd.h"
#include <qt_windows.h>

MainWnd::MainWnd(QWidget *parent) : QWebView(parent)
{
    setMouseTracking(true);
    setAcceptDrops(false);
}

bool MainWnd::nativeEvent(const QByteArray & eventType, void * message, long * result) {
     MSG* msg = reinterpret_cast<MSG*>(message);
     if (msg->message == WM_LBUTTONDOWN) {
         QPoint curPos ( LOWORD(msg->lParam), HIWORD(msg->lParam));

         QRect curRect(0,0,this->width(),82);
         if (curRect.contains(curPos)) {
             ::SendMessageW(HWND(this->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
         }
     }

	 return QWebView::nativeEvent(eventType, message, result);
}

