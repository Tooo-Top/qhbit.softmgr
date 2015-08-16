#include "swmgrapp.h"
#include <QTextCodec>
#include "OSSystemWrapper.h"

void SwmgrApp::docLoadFinish(bool ok) {
    if (ok) {
//        wndMain->setFixedSize(_webPage->mainFrame()->contentsSize());
//        wndMain->page()->mainFrame()->evaluateJavaScript("document.documentElement.style.webkitUserSelect='none';");
    }
}

//==== for UI interface
void SwmgrApp::requestSoftCategoryList() {
    emit updateSoftCategory(_DataModel.getSoftCategory().toVariantList());
}

void SwmgrApp::requestHotList() {
    emit updateHotList(_DataModel.getHotPackages().toVariantList());
}

void SwmgrApp::requestCategoryListByID(QString szCategoryID) {
    // get someone category list
    QMap<QString, QJsonArray>::Iterator curItem = _DataModel.getSoftPackages().find(szCategoryID);
    if ( curItem != _DataModel.getSoftPackages().end() ) {
        emit updateCategoryListForID(szCategoryID, curItem.value().toVariantList());
    }
}

void SwmgrApp::requestPackageInfoByID(QString szCategoryID,QString szPackageID) {
    QMap<QString, QJsonArray>::Iterator curItem = _DataModel.getSoftPackages().find(szCategoryID);
    if (curItem!=_DataModel.getSoftPackages().end()) {
        QJsonArray lstSoftPkg = curItem.value();

        foreach(QJsonValue it,lstSoftPkg) {
            if (it.toObject().value("id").toString().compare(szPackageID,Qt::CaseInsensitive)==0) {
                emit updatePackageInfoByID(it.toObject().toVariantMap());
                break;
            }
        }
    }
}

void SwmgrApp::requestRegisteUser(QString username,QString password,QString email) {
    _user.RegistUser(username,password,email);
    emit updateRegisteUser(QVariant());
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

