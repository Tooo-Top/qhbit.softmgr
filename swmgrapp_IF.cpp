#include "swmgrapp.h"
#include <QTextCodec>
#include "OSSystemWrapper.h"

void SwmgrApp::docLoadFinish(bool ok) {
    if (ok) {
        _DataModel->reqQueryUserStatus();
//        _DataModel->reqQueryAllTaskStatus();
    }
}

void SwmgrApp::testslot(QVariantList var) {
    qDebug()<<var;
}

//==== for UI interface
void SwmgrApp::requestSoftCategoryList() {
    emit updateSoftCategory(_DataModel->getSoftCategory());
}

void SwmgrApp::requestExtraCategoryList(QString szCategoryID,int pageNumber,int count) {
    int pageTotol = 0;
	QVariantList content;
    QVariantList *curCatorgoryList=NULL;
    if (pageNumber<1) { pageNumber=1; }
    if (count<=0) { count=20; }

    if (szCategoryID.compare("hot",Qt::CaseInsensitive)==0) {
		curCatorgoryList = &(_DataModel->getHotPackages());
    }
    else if (szCategoryID.compare("top",Qt::CaseInsensitive)==0){
		curCatorgoryList = &(_DataModel->getTopPackages());
    }
    else {
        emit updateExtraCategoryList(szCategoryID, content,0,pageNumber);
		return ;
    }

    if (curCatorgoryList->size() <=0) {
        emit updateExtraCategoryList(szCategoryID, content,0,pageNumber);
        return ;
    }
    pageTotol = curCatorgoryList->size()/count + ((curCatorgoryList->size()%count)>0 ? 1: 0);
    if (pageNumber>pageTotol) { pageNumber = pageTotol; }

	for (int i = (pageNumber - 1)*count + 0; i < curCatorgoryList->size() && i<((pageNumber - 1)*count + count); i++) {
        content.append(curCatorgoryList->at(i));
    }
    emit updateExtraCategoryList(szCategoryID, content,pageTotol,pageNumber);
}

void SwmgrApp::requestCategoryListByID(QString szCategoryID,int pageNumber,int count) {
    // get someone category list
    int pageTotol = 0;
	QVariantList currentPage;

    if (pageNumber<1) { pageNumber=1; }
    if (count<=0) { count=20; }

	QMap<QString, QVariantList>::Iterator curItem = _DataModel->getSoftPackages().find(szCategoryID);
	if (curItem != _DataModel->getSoftPackages().end()) {
		QVariantList &appData = curItem.value();
        pageTotol = appData.size()/count + ((appData.size()%count)>0 ? 1: 0);
		for (int i = (pageNumber - 1)*count + 0; i<((pageNumber - 1)*count + count) && i< appData.size(); i++) {
            currentPage.append(appData.at(i));
        }
    }
    emit updateCategoryListForID(szCategoryID, currentPage,pageTotol,pageNumber);
}

void SwmgrApp::requestPackageInfoByID(QString szCategoryID,QString szPackageID) {
    qDebug()<<"cateID:"<<szCategoryID<<";packageID:"<<szPackageID;
//	QMap<QString, QVariantList>::Iterator curItem = _DataModel->getSoftPackages().find(szCategoryID);
//	if (curItem != _DataModel->getSoftPackages().end()) {
//        foreach(QVariant it,curItem) {
//            if (it.value("id").toString().compare(szPackageID,Qt::CaseInsensitive)==0) {
//                emit updatePackageInfoByID(it);
//                break;
//            }
//        }
//    }
}

void SwmgrApp::requestRegisteUser(QString username,QString password,QString email) {
	_DataModel->reqRegisteUser(username,password,email);
}

void SwmgrApp::requestLoginUser(QString username,QString password) {
	_DataModel->reqLoginUser(username, password);
}
void SwmgrApp::requestModifyUserInfo(QVariantMap userinfo) {
	_DataModel->reqModifyUserInfo(userinfo);
}

void SwmgrApp::requestCanUninstallPackages() {
    QVariantList jsArray;

	mapSoftwareList &mapSoftwares = _DataModel->getInstalledSoftware();
    for (mapSoftwareList::iterator item = mapSoftwares.begin(); item != mapSoftwares.end();item++) {
        QVariantMap objParameter;
        for (ItemProperty::iterator it = item->second.begin(); it != item->second.end(); it++) {
            objParameter.insert(QString::fromStdString(it->first),QTextCodec::codecForLocale()->toUnicode(it->second.data(), it->second.size()));
        }
		jsArray.append(objParameter);
    }
	emit updateCanUninstallPackages(jsArray);
}

void SwmgrApp::requestStartInstallPackage(QString szCategoryID, QString szPackageID, bool autoInstall) {
	qDebug() << "Category:" << szCategoryID << ";Package:" << szPackageID << ";auto install:" << autoInstall;

	QVariantMap var;
	QMap<QString, QVariantList>::Iterator curItem = _DataModel->getSoftPackages().find(szCategoryID);
	if (curItem != _DataModel->getSoftPackages().end()) {
        foreach(QVariant item,curItem.value()) {
            if (item.toMap().value("id").toString().compare(szPackageID,Qt::CaseInsensitive)==0) {
                var = item.toMap();
		        break;
		    }
		}
	}

    if (var.empty()) {
        qDebug()<<"category:"<<szCategoryID<<",<<package:"<<szPackageID <<"; Not Found!";
        return ;
    }
	QVariantMap task = var;

	task.insert(QString("autoInstall"), QVariant::fromValue(autoInstall));
	_DataModel->reqAddTask(task);
}

void SwmgrApp::requestPausePackage(QString szCategoryID,QString szPackageID) {
    QVariantMap var;
    var.insert("id",QVariant::fromValue(szPackageID));
    var.insert("category",QVariant::fromValue(szCategoryID));
    _DataModel->reqPauseTask(var);
}

void SwmgrApp::requestResumePackage(QString szCategoryID,QString szPackageID) {
    QVariantMap var;
    var.insert("id",QVariant::fromValue(szPackageID));
    var.insert("category",QVariant::fromValue(szCategoryID));
    _DataModel->reqResumeTask(var);
}

void SwmgrApp::requestAllResumePackage() {
    _DataModel->reqResumeAllTask();
}

void SwmgrApp::requestStopDownloadPackage(QString szCategoryID,QString szPackageID){
    QVariantMap var;
    var.insert("id",QVariant::fromValue(szPackageID));
    var.insert("category",QVariant::fromValue(szCategoryID));
    _DataModel->reqRemoveTask(var);
}

void SwmgrApp::requestAllDownloadingTaskPause(){ //Pause all downloading task
    _DataModel->reqPauseAllTask();
}

void SwmgrApp::requestAllDownloadingTaskCancel(){ //Cancel all downloading task
    _DataModel->reqRemoveAllTask();
}

void SwmgrApp::requestOnPageChange(QString pageName) {
    qDebug()<<"requestOnPageChange" << pageName;

    currentPage = pageName;

    if (pageName.compare("index")==0) {
        ;// skip operation
    }
    else if (pageName.compare("login")==0) {
        _DataModel->reqQueryUserStatus();
    }
    else if (pageName.compare("task")==0) {
        _DataModel->reqQueryAllTaskStatus();
    }
    else if (pageName.compare("toolbox")==0) {
        ;
    }
    else if (pageName.compare("uninstall")==0) {
        ;
    }
    else if (pageName.compare("upgrade")==0) {
        ;
    }
    else if (pageName.compare("uprofile")==0) {
        ;
    }
}
