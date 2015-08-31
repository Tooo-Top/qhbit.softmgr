
#include "swmgrapp.h"
#include <QApplication>
#include <QMenu>
#include <QDir>

void SwmgrApp::InitObjects() {
    appTrayIcon = new QIcon();

    trayIconMenu = new QMenu(NULL);
    fullAction = new QAction(QString("Full"), this);
    miniAction = new QAction(QString("Min"), this);
    quitAction = new QAction(QString("Close"), this);
    traySystem = new QSystemTrayIcon(this);

    _DataModel = new DataControl(this);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
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
    myBrowser = new QWebView();

//	wndMain->setContextMenuPolicy(Qt::NoContextMenu);
	myBrowser->setContextMenuPolicy(Qt::NoContextMenu);
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

    QObject::connect(traySystem, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));

    QObject::connect(_DataModel,SIGNAL(sigRequestShow()),this,SLOT(showFullWnd()));
    QObject::connect(_DataModel,SIGNAL(sigCrash()),this,SLOT(appquit()));

    QObject::connect(_DataModel,SIGNAL(updateLoginUser(QVariantMap)),this,SIGNAL(updateLoginUser(QVariantMap)));
    QObject::connect(_DataModel,SIGNAL(updateRegisteUser(QVariantMap)),this,SIGNAL(updateRegisteUser(QVariantMap)));
    QObject::connect(_DataModel,SIGNAL(updateModifyUserInfo(QVariantMap)),this,SIGNAL(updateModifyUserInfo(QVariantMap)));

	QObject::connect(_DataModel, SIGNAL(updateAllTaskStatus(QVariantList)), this, SIGNAL(updateRunningTasks(QVariantList)));
    QObject::connect(_DataModel, SIGNAL(updateTaskStatus(QVariantMap)),this,SIGNAL(updateTaskInfo(QVariantMap)));
    QObject::connect(_DataModel, SIGNAL(updateTaskDownloadProgress(QVariantMap)),this,SIGNAL(updateDownloadProgress(QVariantMap)));

    QObject::connect(_DataModel, SIGNAL(updateTaskDownloadProgress(QVariantMap)), this, SLOT(testslot(QVariantMap)));
}

void SwmgrApp::InitDataModel() {
    _DataModel->initAll();
}

void SwmgrApp::InitTray() {
    traySystem->setIcon(*appTrayIcon);
    traySystem->setContextMenu(trayIconMenu);
    traySystem->show();
}

void SwmgrApp::InitWnd() {
    wndMain->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
    wndMain->setMouseTracking(true);
    wndMain->setFixedSize(963, 595);
    _webPage->mainFrame()->load(QUrl::fromUserInput(GLOBAL::_DY_DIR_RUNNERSELF + "/lewang/Index.html"));
    QObject::connect(_webPage->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(initWebViewHost()));
    QObject::connect(_webPage->mainFrame(), SIGNAL(loadFinished(bool)), this, SLOT(docLoadFinish(bool)));
    wndMain->show();

    myBrowser->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint/* | Qt::Popup*/);
    myBrowser->setAttribute(Qt::WA_TranslucentBackground, true);
    myBrowser->setWindowOpacity(0.5f);
}

void SwmgrApp::appquit() {
	wndMain->hide();
	wndMain->close();
	_webPage->deleteLater();
    wndMain->deleteLater();
	QWebSettings::globalSettings()->clearMemoryCaches();
    _DataModel->unInit();
	qApp->quit();
}

void SwmgrApp::showFullWnd() {
    if (wndMain->isVisible())
        wndMain->hide();
    else {
        wndMain->show();
		wndMain->activateWindow();
    }
}

void SwmgrApp::showMiniWnd() {
//    if (wndMain->isVisible())
//        wndMain->hide();
    // show mini widget
}

void SwmgrApp::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason==QSystemTrayIcon::DoubleClick) {
        showFullWnd();
    }
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
 * Use explorer open local disk folder
 * @brief SwmgrApp::execOpenLocalFolder
 * @param localAddress
 */
void SwmgrApp::execOpenLocalFolder(QString localAddress){
    QDesktopServices::openUrl(QUrl::fromUserInput(localAddress));
}


/**
 * Use explorer open local download folder
 * @brief SwmgrApp::execOpenLocalDownloadFolder
 */
void SwmgrApp::execOpenLocalDownloadFolder(){
	QDir dir;
	QString defaultRepository = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Repository") + QDir::separator();
	defaultRepository = QDir::toNativeSeparators(defaultRepository);
	defaultRepository = getSettingParameter(QString("Repository"), defaultRepository);
	dir.mkpath(defaultRepository);

	QDesktopServices::openUrl(QUrl::fromUserInput(defaultRepository));
}

/**
 * Use self webkit container pop open a qrc|file|http url
 * @brief SwmgrApp::execOpenPopBrowser
 * @param urlAddress
 * @param windowWidth
 * @param windowHeight
 */
void SwmgrApp::execOpenPopBrowser(QString urlAddress, int windowWidth, int windowHeight){
    if (myBrowser) {
        if (urlAddress.count()>7 && (urlAddress.left(7).compare("http://")==0 || urlAddress.left(8).compare("https://")==0)) {
            myBrowser->page()->mainFrame()->load(QUrl::fromUserInput(urlAddress));
        }
        else {
            myBrowser->page()->mainFrame()->load(QUrl::fromUserInput(GLOBAL::_DY_DIR_RUNNERSELF + "/lewang/" + urlAddress));
        }

        myBrowser->page()->view()->setGeometry((qApp->desktop()->width() - windowWidth) / 2, (qApp->desktop()->height() - windowHeight) / 2, windowWidth, windowHeight);
		myBrowser->page()->view()->show();
    }
}
