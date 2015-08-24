#include "swmgrapp.h"
#include <QTextCodec>
#include "OSSystemWrapper.h"

void SwmgrApp::docLoadFinish(bool ok) {
    if (ok) {
        _DataModel->reqQueryUserState();
        _DataModel->checkAllTaskInfo();
    }
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
	QMap<QString, QVariantList>::Iterator curItem = _DataModel->getSoftPackages().find(szCategoryID);
	if (curItem != _DataModel->getSoftPackages().end()) {
        //foreach(QVariant it,curItem) {
        //    if (it.value("id").toString().compare(szPackageID,Qt::CaseInsensitive)==0) {
        //        emit updatePackageInfoByID(it);
        //        break;
        //    }
        //}
    }
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
	_DataModel->StartInstallPackage(szCategoryID, szPackageID, autoInstall);
}
