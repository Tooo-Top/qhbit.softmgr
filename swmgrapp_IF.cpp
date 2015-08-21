#include "swmgrapp.h"
#include <QTextCodec>
#include "OSSystemWrapper.h"

void SwmgrApp::checkAllTaskInfo() {
    QJsonArray var;
    foreach(LPDowningTaskObject taskObject, _PendingTasks.getTasks().values()) {
        QJsonObject it;
        it["id"]=taskObject->id;
        it["name"]=taskObject->name;
        it["category"]=taskObject->category;
        it["status"]=taskObject->status;
        it["percent"]=taskObject->percent;
        var.append(it);
    }

    emit updateRunningTasks(var.toVariantList());
}

void SwmgrApp::docLoadFinish(bool ok) {
    if (ok) {
//        wndMain->setFixedSize(_webPage->mainFrame()->contentsSize());
//        wndMain->page()->mainFrame()->evaluateJavaScript("document.documentElement.style.webkitUserSelect='none';");
		emit updateLoginUser(_user.toJsonObject().toVariantMap());
        checkAllTaskInfo();
    }
}

//==== for UI interface
void SwmgrApp::requestSoftCategoryList() {
    emit updateSoftCategory(_DataModel.getSoftCategory().toVariantList());
}

void SwmgrApp::requestExtraCategoryList(QString szCategoryID,int pageNumber,int count) {
    int pageTotol = 0;
	QJsonArray content;
    QJsonArray *curCatorgoryList=NULL;
    if (pageNumber<1) { pageNumber=1; }
    if (count<=0) { count=20; }

    if (szCategoryID.compare("hot",Qt::CaseInsensitive)==0) {
        curCatorgoryList = &(_DataModel.getHotPackages());
    }
    else if (szCategoryID.compare("top",Qt::CaseInsensitive)==0){
        curCatorgoryList = &(_DataModel.getTopPackages());
    }
    else {
        emit updateExtraCategoryList(szCategoryID, content.toVariantList(),0,pageNumber);
		return ;
    }

    if (curCatorgoryList->size() <=0) {
        emit updateExtraCategoryList(szCategoryID, content.toVariantList(),0,pageNumber);
        return ;
    }
    pageTotol = curCatorgoryList->size()/count + ((curCatorgoryList->size()%count)>0 ? 1: 0);
    if (pageNumber>pageTotol) { pageNumber = pageTotol; }

	for (int i = (pageNumber - 1)*count + 0; i < curCatorgoryList->size() && i<((pageNumber - 1)*count + count); i++) {
        content.append(curCatorgoryList->at(i));
    }
    emit updateExtraCategoryList(szCategoryID, content.toVariantList(),pageTotol,pageNumber);
}

void SwmgrApp::requestCategoryListByID(QString szCategoryID,int pageNumber,int count) {
    // get someone category list
    int pageTotol = 0;
	QJsonArray currentPage;

    if (pageNumber<1) { pageNumber=1; }
    if (count<=0) { count=20; }

    QMap<QString, QJsonArray>::Iterator curItem = _DataModel.getSoftPackages().find(szCategoryID);
    if ( curItem != _DataModel.getSoftPackages().end() ) {
        QJsonArray &appData = curItem.value();
        pageTotol = appData.size()/count + ((appData.size()%count)>0 ? 1: 0);
		for (int i = (pageNumber - 1)*count + 0; i<((pageNumber - 1)*count + count) && i< appData.size(); i++) {
            currentPage.append(appData.at(i));
        }
    }
    emit updateCategoryListForID(szCategoryID, currentPage.toVariantList(),pageTotol,pageNumber);
}

void SwmgrApp::requestPackageInfoByID(QString szCategoryID,QString szPackageID) {
    QMap<QString, QJsonArray>::Iterator curItem = _DataModel.getSoftPackages().find(szCategoryID);
    if (curItem!=_DataModel.getSoftPackages().end()) {
        foreach(QJsonValue it,curItem.value()) {
            if (it.toObject().value("id").toString().compare(szPackageID,Qt::CaseInsensitive)==0) {
                emit updatePackageInfoByID(it.toObject().toVariantMap());
                break;
            }
        }
    }
}

void SwmgrApp::requestRegisteUser(QString username,QString password,QString email) {
    _user.RegistUser(username,password,email);
    emit updateRegisteUser(_user.toJsonObject().toVariantMap());
}

void SwmgrApp::requestLoginUser(QString username,QString password) {
    _user.UserLogin(username,password);
    emit updateLoginUser(_user.toJsonObject().toVariantMap());
}

void SwmgrApp::requestCanUninstallPackages() {
    QJsonArray jsArray;

    mapSoftwareList mapSoftwares;
    OSSystemWrapper::Instance()->GetSystemInstalledSoftware(mapSoftwares,0);
    for (mapSoftwareList::iterator item = mapSoftwares.begin(); item != mapSoftwares.end();item++) {
        QJsonObject objParameter;
        for (ItemProperty::iterator it = item->second.begin(); it != item->second.end(); it++) {
            objParameter[QString::fromStdString(it->first)] = QTextCodec::codecForLocale()->toUnicode(it->second.data(), it->second.size());
        }
        jsArray.append(objParameter);
    }
    emit updateCanUninstallPackages(jsArray.toVariantList());
}

void SwmgrApp::requestStartInstallPackage(QString szCategoryID, QString szPackageID, bool autoInstall) {
	qDebug() << "Category:" << szCategoryID << ";Package:" << szPackageID << ";auto install:" << autoInstall;
    QJsonObject installer;
    QMap<QString, QJsonArray>::Iterator curItem = _DataModel.getSoftPackages().find(szCategoryID);
    if (curItem!=_DataModel.getSoftPackages().end()) {
        foreach(QJsonValue it,curItem.value()) {
            if (it.toObject().value("id").toString().compare(szPackageID,Qt::CaseInsensitive)==0) {
                installer["id"]=it.toObject().value("id").toString();
                installer["catid"] = it.toObject().value("category").toString();
                installer["launchName"] = it.toObject().value("name").toString() + ".lnk";
                emit sigInstaller(installer);
                break;
            }
        }
    }
}
