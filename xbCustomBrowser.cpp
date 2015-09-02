#include "xbCustomBrowser.h"

xbCustomBrowser::xbCustomBrowser(QWidget *parent) : QWebView(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
    this->setStyleSheet("background:transparent");
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setContextMenuPolicy(Qt::NoContextMenu);
//    QObject::connect(page(), SIGNAL(windowCloseRequested()), this, SLOT(requestClose()));
    QObject::connect(page(), SIGNAL(windowCloseRequested()), this, SLOT(close()));
}

void xbCustomBrowser::requestClose() {
    close();
    setPage(0);
//	deleteLater();
}
