
#include "swmgrapp.h"
#include <QApplication>
#include <QMenu>
#include <QDir>
#include <QDesktopServices>

void SwmgrApp::InitObjects() {
    appTrayIcon = new QIcon();

    trayIconMenu = new QMenu(NULL);
    fullAction = new QAction(QString("Full"), this);
    miniAction = new QAction(QString("Min"), this);
    quitAction = new QAction(QString("Close"), this);
    traySystem = new QSystemTrayIcon(this);
    srvLaunchInst = new QLocalServer(this);
    pollDownloadTaskObject = new QTimer(this);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);
    QWebSettings::globalSettings()->setMaximumPagesInCache(0);
    QWebSettings::globalSettings()->setObjectCacheCapacities(0, 0, 0);
    QWebSettings::globalSettings()->setOfflineStorageDefaultQuota(0);
    QWebSettings::globalSettings()->setOfflineWebApplicationCacheQuota(0);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);
    QString strCacheDir= GetProgramProfilePath(QString("xbsoftMgr")) + "\\BrowserCache";
    QDir d(strCacheDir);
    if(!d.exists()){
        d.mkpath(strCacheDir);
    }
    QWebSettings::globalSettings()->enablePersistentStorage(strCacheDir);


    wndMain = new MainWnd();
    _webPage = new QWebPage();
    wndMain->setPage(_webPage);
}

void SwmgrApp::InitIcons() {
    appTrayIcon->addFile(":/xbmgr.ico");
}

void SwmgrApp::InitMenuActions() {
    trayIconMenu->addAction(fullAction);
    trayIconMenu->addAction(miniAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
}

void SwmgrApp::InitSlots() {
    QObject::connect(quitAction, SIGNAL(triggered(bool)), this, SLOT(appquit()));
    QObject::connect(fullAction, SIGNAL(triggered(bool)), this, SLOT(showFullWnd()));
    QObject::connect(miniAction, SIGNAL(triggered(bool)), this, SLOT(showMiniWnd()));
    QObject::connect(srvLaunchInst, SIGNAL(newConnection()), this, SLOT(launchInstall()));
    QObject::connect(this, SIGNAL(sigInstaller(QJsonObject)), SLOT(addInstaller(QJsonObject)));
    QObject::connect(pollDownloadTaskObject, SIGNAL(timeout()), this,SLOT(downloadPoll()));
}

void SwmgrApp::InitTray() {
    traySystem->setIcon(*appTrayIcon);
    traySystem->setContextMenu(trayIconMenu);
    traySystem->show();
}

void SwmgrApp::InitWnd() {
    //wndFull = new Widget(NULL);
    wndMain->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
    wndMain->setMouseTracking(true);
    wndMain->setFixedSize(963, 595);
    _webPage->mainFrame()->load(QUrl::fromUserInput(GLOBAL::_DY_DIR_RUNNERSELF + "/lewang/Index.html"));
    //_webPage->mainFrame()->load(QUrl::fromUserInput("qrc:/index.html"));
    _webPage->triggerAction(QWebPage::Reload,false);
    QObject::connect(_webPage->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(initWebViewHost()));
    QObject::connect(_webPage->mainFrame(), SIGNAL(loadFinished(bool)), this, SLOT(docLoadFinish(bool)));
    wndMain->show();
}

void SwmgrApp::appquit() {
    wndMain->close();
    _webPage->deleteLater();
    wndMain->deleteLater();
    QWebSettings::globalSettings()->clearMemoryCaches();
    qApp->quit();
//	QCoreApplication::exit(0);
}

void SwmgrApp::showFullWnd() {
    if (wndMain->isVisible())
        wndMain->hide();
    else {
        wndMain->show();
    }
}

void SwmgrApp::showMiniWnd() {
//    if (wndMain->isVisible())
//        wndMain->hide();
    // show mini widget
}

void SwmgrApp::initWebViewHost() {
    wndMain->page()->mainFrame()->addToJavaScriptWindowObject("DYBC",this);
}


/**
 * Use system default browser open url address
 * @brief SwmgrApp::openSystemBrowser
 * @param urlAddress
 */
void SwmgrApp::execOpenSystemBrowser(QString urlAddress){
    QDesktopServices::openUrl(QUrl::fromUserInput(urlAddress));
}


/**
 * Use self webkit container pop open a qrc|file|http url
 * @brief SwmgrApp::execOpenPopBrowser
 * @param urlAddress
 * @param windowWidth
 * @param windowHeight
 */
void SwmgrApp::execOpenPopBrowser(QString urlAddress, int windowWidth, int windowHeight){

}