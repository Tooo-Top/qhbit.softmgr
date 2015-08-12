#ifndef MAINWND_H
#define MAINWND_H

#include <QWebView>
#include <QMouseEvent>
#include <QByteArray>

class MainWnd : public QWebView
{
    Q_OBJECT
public:
    explicit MainWnd(QWidget *parent = 0);
protected:
/*    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);*/
    virtual bool nativeEvent(const QByteArray & eventType, void * message, long * result);
protected:
//    QPoint _pos;
signals:

public slots:
};

#endif // MAINWND_H
