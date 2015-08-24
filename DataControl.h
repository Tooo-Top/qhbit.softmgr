#ifndef DATACONTROL_H
#define DATACONTROL_H

#include <QObject>

#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QLocalServer>
#include <QLocalSocket>

#include <QJsonObject>

#include "ConfOperation.h"
#include "SoftwareList.h"

#include "OSSystemWrapper.h"
#include "PackageRunner.h"
#include "UserInfo.h"

#include "TaskManager.h"
#include "UserInfoManager.h"

class DataControl : public QObject
{
    Q_OBJECT
public:
    DataControl(QObject *parent=0);

protected:
	// -----------------------
    QVariantList _lstSoftCategory;    //category list
    QVariantList _lstSoftTopPackages;
    QVariantList _lstSoftHotPackages;
    QMap<QString,QVariantList> _mapSoftPackages; //package list by category

	// -----------------------
    mapSoftwareList _mapInstalledSoftwares;
protected:
    TaskManager *_TaskRunner;
    UserInfoManager* _UserInRunner;
    //
    QVariantMap  _setting;        // program setting
    UserInfo     _user;           // user
    /* pending and downing */
    PackageRunner _PendingTasks;  // use someone event poll this map for monitor task object status

    // for desk shelllink launch
    QLocalServer *srvLaunchInst;

public:
    QVariantList &getSoftCategory();
    QVariantList &getTopPackages();
    QVariantList &getHotPackages();
    QMap<QString,QVariantList> &getSoftPackages();

    mapSoftwareList &getInstalledSoftware();
    TaskManager *getTaskManager();
    PackageRunner &getPackageRunner();
public:
	bool initSoftCategory();
	bool initSoftPackages();
	bool initTopPackages();
	bool initHotPackages();

	bool initAll();

    void startUserService();
    void startTaskService();

    void reqLoginUser(QString username,QString password);
    void reqRegisteUser(QString username,QString password,QString email);
    void reqModifyUserInfo(QVariantMap userinfo);
    void reqQueryUserState();
public:
    void LoadSettingProfile();
    void SaveSettingProfile();
    QString getSettingParameter(QString name, QString defaultValue);
public:
    static void NoticeMain(QObject *parent, QVariantMap &jsItem);
protected:
    void InitNoticeServer();
protected slots:
    void launchInstall();
signals:
    void sigRequestShow();

    void sigInstaller(QJsonObject);
    void updateRunningTasks(QVariantList swCategory);

    // response
    void updateLoginUser(QVariantMap userinfo);
    void updateRegisteUser(QVariantMap userinfo);
    void updateModifyUserInfo(QVariantMap userinfo);

    // request
    void sigLoginUser(QString username,QString password);
    void sigRegisteUser(QString username,QString password,QString email);
    void sigModifyUserInfo(QVariantMap userinfo);
    void sigQueryUserState();

public slots:
    void InstalledSoftwareChanged();
    void addInstaller(QJsonObject installer);
    void checkAllTaskInfo();
	void StartInstallPackage(QString szCategoryID, QString szPackageID, bool autoInstall);
};

#endif // DATACONTROL_H
