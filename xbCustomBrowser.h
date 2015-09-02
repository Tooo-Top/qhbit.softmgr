#ifndef XBCUSTOMBROWSER_H
#define XBCUSTOMBROWSER_H

#include <QWidget>
#include <QWebView>

class xbCustomBrowser : public QWebView
{
    Q_OBJECT
public:
    explicit xbCustomBrowser(QWidget *parent = 0);

signals:

public slots:
    void requestClose();
protected:
};

#endif // XBCUSTOMBROWSER_H
