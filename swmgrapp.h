#ifndef SWMGRAPP_H
#define SWMGRAPP_H
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib") 

#include <QObject>
#include <QAction>
#include <QSystemTrayIcon>
#include <QProcessEnvironment>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <QWebView>
#include <QWebPage>
#include <QWebFrame>

#include "global.h"

#include "ConfOperation.h"
#include "DataControl.h"

#include "MainWnd.h"

class SwmgrApp : public QObject
{
    Q_OBJECT
public:
    explicit SwmgrApp(QObject *parent = 0);
	~SwmgrApp();
public:
    static SwmgrApp *Instance();
	static const QString &GetEnvVar();
	static const QString &GetCompanyName();
	static const QString &GetSoftwareName();

	static QString GetAppDataPath(QString szCompany);
	static QString GetProgramProfilePath(QString name);
	static QString GetFilePathFromFile(QString szFile);
	static QString GetCookieFile();
	static QString GetUserLoginUrl();
	static QString GetUserRegisteUrl();
	// ------------------------
    static void NoticeMain(QObject *parent, QVariantMap &jsItem);
public:
    QString getSettingParameter(QString name, QString defaultValue);

public:
	// ------------------------
	BOOL InitAppEnv();
	void InitDir(QString szAppDir);
	BOOL InitCurl();

protected:
	void InitObjects();
    void InitIcons();
    void InitMenuActions();
    void InitSlots();
    void InitDataModel();
    void InitTray();
    void InitWnd();
	void DumpEnv();

protected:
    QSystemTrayIcon *traySystem;
    QMenu *trayIconMenu;
    // -----------------------
    QAction *quitAction;
    QAction *fullAction;
    QAction *miniAction;
    // -----------------------
    QIcon   *appTrayIcon;

    // -----------------------
    MainWnd *wndMain;
	QWebPage *_webPage;

	QWebView *myBrowser;
	// -----------------------

    // -----------------------
    DataControl *_DataModel;
	// -----------------------
	BOOL m_bCurlStatus;

    QString currentPage;
protected slots:
    // operation
    void monitorProf();
    void initWebViewHost();
    void docLoadFinish(bool);
    void testslot(QVariantMap var);
public slots:
    // system control ex:show close hide
    void appquit();
    void showFullWnd();
    void showMiniWnd();
    void trayActivated(QSystemTrayIcon::ActivationReason);

    //msic js bind signal
    void execOpenSystemBrowser(QString urlAddress);//use system default browser open a http://website
    void execOpenPopBrowser(QString urlAddress, int windowWidth, int windowHeight);//use self webkit container open a qrc|file|http url
    void execOpenLocalFolder(QString localAddress);//open local disk folder on explorer
    void execOpenLocalDownloadFolder();//open local disk download folder on explorer

// For UI interface begin
signals:
    void updateSoftCategory(QVariantList swCategory);//for software category
    void updateExtraCategoryList(QString szCategoryID,QVariantList swCategory,int pageTotal,int pageNumber); //for top/hot/other... list
    void updateCategoryListForID(QString szCategoryID,QVariantList swCategory,int pageTotal,int pageNumber); //for someone category list
    void updatePackageInfoByID(QVariantMap swObject); //for someone package info

    //about user
    void updateRegisteUser(QVariantMap userinfo);
    void updateLoginUser(QVariantMap userinfo);
    void updateModifyUserInfo(QVariantMap userinfo);

    //soft package operation
    void updateRunningTasks(QVariantList swCategory);// all package task status
    void updateTaskInfo(QVariantMap swPackageInfo);  // someone task status
    void updateDownloadProgress(QVariantMap swTaskProcess );//down load progress

    void updateUpgradePackages(QVariantList swCategory);
	void updateCanUninstallPackages(QVariantList swCategory);

public slots:
    //about software
    void requestSoftCategoryList();
    void requestExtraCategoryList(QString szCategoryID,int pageNumber=0,int count=0);//fetch top/hot/other category packages list
    void requestCategoryListByID(QString szCategoryID,int pageNumber=0,int count=0);
    void requestPackageInfoByID(QString szCategoryID,QString szPackageID);

    //about user
    void requestRegisteUser(QString username,QString password,QString email);
    void requestLoginUser(QString username,QString password);
	void requestModifyUserInfo(QVariantMap userinfo);

    //soft package operation
	void requestStartInstallPackage(QString szCategoryID, QString szPackageID, bool autoInstall); // software package download and install ( auto install ??)
    void requestBatStartInstallPackage(QVariantList lstPackage){}  // bat install
    void requestPausePackage(QString szCategoryID,QString szPackageID); //pause someone package
    void requestResumePackage(QString szCategoryID,QString szPackageID); //resume someone package
    void requestAllResumePackage(); //resume all package
    void requestStopDownloadPackage(QString szCategoryID,QString szPackageID); // remove processing task for download/installtion

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++^
    void requestAllDownloadingTaskPause(); //Pause all downloading task
    void requestAllDownloadingTaskCancel(); //Cancel all downloading task
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++$

    void requestCanUpdatePackages(); //can be update pacakge list
	void requestCanUninstallPackages(); //can be uninstall pacakge list
    void requestDoUninstall(QString uninstallID);
    void requestOnPageChange(QString pageName); //Webkit (dynamic dom page) change to {pageName}

    // For UI interface end
};

#endif // SWMGRAPP_H
