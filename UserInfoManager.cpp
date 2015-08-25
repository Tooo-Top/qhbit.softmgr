#include "UserInfoManager.h"
#include <QVariantMap>
#include "DataControl.h"
#include "UserInfo.h"

UserInfoManager::UserInfoManager(QObject *parent) : QThread(parent)
{
    pDataControl=NULL;
    pUserInfo = NULL;
}

void UserInfoManager::SetObjects(DataControl *dataControl,UserInfo *userInfo) {
    pDataControl = dataControl;
    pUserInfo = userInfo;
    pUserInfo->moveToThread(this);
}

void UserInfoManager::run() {
    if (pUserInfo!=NULL && pDataControl!=NULL) {
        // request
        QObject::connect(pDataControl,SIGNAL(sigRegisteUser(QString,QString,QString)),pUserInfo,SLOT(RegistUser(QString,QString,QString)),Qt::QueuedConnection);
        QObject::connect(pDataControl,SIGNAL(sigLoginUser(QString,QString)),pUserInfo,SLOT(UserLogin(QString,QString)),Qt::QueuedConnection);
        QObject::connect(pDataControl,SIGNAL(sigModifyUserInfo(QVariantMap)),pUserInfo,SLOT(ModifyUserInfo(QVariantMap)),Qt::QueuedConnection);
        QObject::connect(pDataControl,SIGNAL(sigQueryUserState()),pUserInfo,SLOT(QueryUserInfo()),Qt::QueuedConnection);

        //response
        QObject::connect(pUserInfo,SIGNAL(signalRegisteUser(QVariantMap)),pDataControl,SIGNAL(updateRegisteUser(QVariantMap)),Qt::QueuedConnection);
        QObject::connect(pUserInfo,SIGNAL(signalLoginUser(QVariantMap)),pDataControl,SIGNAL(updateLoginUser(QVariantMap)),Qt::QueuedConnection);
        QObject::connect(pUserInfo,SIGNAL(signalModifyUserInfo(QVariantMap)),pDataControl,SIGNAL(updateModifyUserInfo(QVariantMap)),Qt::QueuedConnection);

        pUserInfo->serializeUserInfo();        
        QThread::exec();
    }
    else{
        QThread::exit(1);
    }
}
