#include "MainWnd.h"
#include <qt_windows.h>


MainWnd::MainWnd(QWidget *parent) : QWebView(parent)
{
    setMouseTracking(true);
}
/*
void MainWnd::mousePressEvent(QMouseEvent * event) {
    if (event->buttons() & Qt::LeftButton) {
        _pos = event->pos();
    }
	else {
		QWebView::mousePressEvent(event);
	}
}

void MainWnd::mouseMoveEvent(QMouseEvent * event) {
    if (event->buttons() & Qt::LeftButton) {
        QPoint _cPos = event->pos();
        QPoint wndPos = this->pos();
        wndPos.setX(wndPos.x() + (_cPos.x()-_pos.x()));
        wndPos.setY(wndPos.y() + (_cPos.y()-_pos.y()));
		this->move(wndPos);
        _pos = _cPos;
    }
	else {
		QWebView::mouseMoveEvent(event);
	}
}

void MainWnd::mouseReleaseEvent(QMouseEvent * event) {
    if (event->buttons() & Qt::LeftButton) {
//        this->move(event->pos());
    }
	else {
		QWebView::mouseReleaseEvent(event);
	}
}
*/

bool MainWnd::nativeEvent(const QByteArray & eventType, void * message, long * result) {
     MSG* msg = reinterpret_cast<MSG*>(message);
     if (msg->message == WM_NCHITTEST) {
         *result = HTCAPTION;
         return true;
     }
     return false;
}
