#ifndef SWMGRAPP_H
#define SWMGRAPP_H
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib") 
#include <QObject>
#include <QTimer>
#include <QAction>
#include <QSystemTrayIcon>
#include <QProcessEnvironment>
#include <QLocalServer>
#include <QLocalSocket>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include "global.h"
#include "DownWrapper.h"

#include "ConfOperation.h"
#include "SoftwareList.h"
#include "DataControl.h"
#include "PackageRunner.h"

#include "UserInfo.h"
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
	static void NoticeMain(QObject *parent, QJsonObject &jsItem);
public:
	// ------------------------
	BOOL InitAppEnv();
	void InitDir(QString szAppDir);
	BOOL InitCurl();
	BOOL InitMiniXL();
protected:
	void InitObjects();
    void InitIcons();
    void InitMenuActions();
    void InitSlots();
    void InitTray();
    void InitWnd();
	void InitNoticeServer();
	void StartPoll();
	void UninitEnv();
	void DumpEnv();
protected:
	// program setting
	void LoadSettingProfile();
	void SaveSettingProfile();
	QString getSettingParameter(QString name, QString defaultValue);

	// XL mini
	DownWrapper* LoadDll();
	void UnloadDll(DownWrapper** Wapper);
	// ShellLink
	BOOL CreateShellLink(QString szTargetExec, QString szID, QString szCategory, QString szLnkName, QString szIconName, QString szDescription);
protected:
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
    QWebView *myBrowser;
	QWebPage *myBrowserPage;
    QWebPage *_webPage;
	// -----------------------
	QLocalServer *srvLaunchInst;
	QTimer *pollDownloadTaskObject;
	// -----------------------

	/* pending and downing */
	PackageRunner _PendingTasks;  // use someone event poll this map for monitor task object status
	// -----------------------
	DataControl _DataModel;
	// -----------------------
	DownWrapper* _pWapper;  // xunlei mini
	QJsonObject  _setting;  // program setting
	BOOL m_bCurlStatus;

	UserInfo _user;
signals:
	void sigInstaller(QJsonObject installer);
protected slots:
    void addInstaller(QJsonObject installer);

protected slots:
    // task
    void downloadPoll();
    // operation
    void monitorProf();
    void launchInstall();
    void initWebViewHost();
    void docLoadFinish(bool);
public slots:
    // system control ex:show close hide
    void appquit();
    void showFullWnd();
    void showMiniWnd();

    //msic js bind signal
    void execOpenSystemBrowser(QString urlAddress);//use system default browser open a http://website
    void execOpenPopBrowser(QString urlAddress, int windowWidth, int windowHeight);//use self webkit container open a qrc|file|http url

    //misc operation
    //void openSystemBrowser(QUrl urlAddress){}
    //void openWebkitWindow(QUrl urlAddress

// For UI interface begin
signals:
    void updateSoftCategory(QVariantList swCategory);//for software category
    void updateExtraCategoryList(QString szCategoryID,QVariantList swCategory,int pageTotal,int pageNumber); //for top/hot/other... list
    void updateCategoryListForID(QString szCategoryID,QVariantList swCategory,int pageTotal,int pageNumber); //for someone category list
    void updatePackageInfoByID(QVariantMap swObject); //for someone package info

    //about user
    void updateRegisteUser(QVariant userinfo);
    void updateLoginUser(QVariant userinfo);
    void updateModifyUserInfo(QVariant userinfo);

    //soft package operation
    void updateRunningTasks(QVariantList swCategory);// all package task status
    void updateTaskInfo(QVariantMap swPackageInfo);  // someone task status
    void updateDownloadProgress(QString szCategoryID,QString szPackageID,float fPercent);//down load progress
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
    void requestModifyUserInfo(QVariant userinfo){}

    //soft package operation
    void requestStartInstallPackage(QString szCategoryID,QString szPackageID,bool autoInstall){} // software package download and install ( auto install ??)
    void requestBatStartInstallPackage(QVariantList lstPackage){}  // bat install
    void requestPausePackage(QString szCategoryID,QString szPackageID){} //pause someone package
    void requestResumePackage(QString szCategoryID,QString szPackageID){} //resume someone package
    void requestAllResumePackage(){} //resume all package
    void requestStopDownloadPackage(QString szCategoryID,QString szPackageID){} // remove processing task for download/installtion

	void requestCanUpdatePackages(){} //can be update pacakge list
	void requestCanUninstallPackages(); //can be uninstall pacakge list

    void requestOnPageChange(QString pageName); //Webkit (dynamic dom page) change to {pageName}

    // For UI interface end
};

#endif // SWMGRAPP_H
