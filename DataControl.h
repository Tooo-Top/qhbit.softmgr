#ifndef DATACONTROL_H
#define DATACONTROL_H

#include <QObject>

#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <QLocalServer>
#include <QLocalSocket>
#include <QFileSystemWatcher>

#include "OSSystemWrapper.h"

#include "ConfOperation.h"

#include "UserInfo.h"
#include "UserInfoManager.h"

#include "PackageRunner.h"
#include "TaskManager.h"

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
    QFileSystemWatcher* _cofigureWatcher;
    TaskManager* _TaskRunner;
    UserInfoManager* _UserInRunner;
	QVariantList mapLaunchTask;
    // programe setting

    //QFileSystemWatcher
    QVariantMap  _setting;        // program setting
    // user info
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

public:
	bool initSoftCategory();
	bool initSoftPackages();
	bool initTopPackages();
	bool initHotPackages();

	bool initAll();

    void startUserService();
    void startTaskService();
    void unInit();
    void driveLaunchTasks();
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

    // others
signals:
    void sigRequestShow();
    void sigCrash();
protected slots:
    void InstalledSoftwareChanged();
    void configureChanged(QString path);

    // for task
signals:
    // response task operation
    void updateAllTaskStatus(QVariantList taskStatus);
    void updateTaskStatus(QVariantMap taskStatus);
    void updateTaskDownloadProgress(QVariantMap swTaskProcess );

    // request task operation
    void sigQueryAllTaskStatus();

    void sigAddTask(QVariantMap task,bool autoInstall);
    void sigAddTasks(QVariantList tasks);

    void sigPauseTask(QVariantMap task);
    void sigPauseAllTask();

    void sigResumeTask(QVariantMap task);
    void sigResumeAllTask();

    void sigRemoveTask(QVariantMap task);
    void sigRemoveAllTask();

public slots:
    void reqQueryAllTaskStatus();

	void reqAddTask(QVariantMap task, bool autoInstall);
    void reqAddTasks(QVariantList tasks);

    void reqPauseTask(QVariantMap task);
    void reqPauseAllTask();

    void reqResumeTask(QVariantMap task);
    void reqResumeAllTask();

    void reqRemoveTask(QVariantMap task);
    void reqRemoveAllTask();

    // for user
signals:
    // response user operation
    void updateLoginUser(QVariantMap userinfo);
    void updateRegisteUser(QVariantMap userinfo);
    void updateModifyUserInfo(QVariantMap userinfo);

    // request user operation
    void sigLoginUser(QString username,QString password);
    void sigRegisteUser(QString username,QString password,QString email);
    void sigModifyUserInfo(QVariantMap userinfo);

    void sigQueryUserState();
public slots:
    void reqLoginUser(QString username,QString password);
    void reqRegisteUser(QString username,QString password,QString email);
    void reqModifyUserInfo(QVariantMap userinfo);
    void reqQueryUserStatus();
};

#endif // DATACONTROL_H
