#ifndef SWMGRAPP_H
#define SWMGRAPP_H
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib") 
#include <QObject>
#include <QAction>
#include <QSystemTrayIcon>
#include <QProcessEnvironment>
#include <QLocalServer>
#include <QLocalSocket>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include "global.h"
#include "widget.h"
#include "DownWrapper.h"

#include "ConfOperation.h"
#include "SoftwareList.h"
#include "DataControl.h"
#include "PackageRunner.h"

#include "UserInfo.h"

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
//  Widget  *wndFull;
//	QWidget *wndFloat;
//	QWidget *wndMini;
    QWebView *wndWebkit;
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
    void putSoftCategory(QVariantList swCategory);
protected slots:
    // task
    void addInstaller(QJsonObject installer);
    void downloadPoll();
    // operation
    void monitorProf();
    void launchInstall();

    void dumpInfo(QVariantList swCategory);
public slots:
    void requestSoftCategoryList();
    // UI
    void appquit();
    void showFullWnd();
    void showMiniWnd();
    void docLoadFinish(bool);

};

#endif // SWMGRAPP_H
