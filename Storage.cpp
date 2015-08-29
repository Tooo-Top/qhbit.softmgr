#include "Storage.h"
#include <QtDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

Storage::Storage(QObject *parent) : QObject(parent)
{

}

void Storage::getSettingFromFile(QString szFile, QVariantMap &setting) {
    QByteArray fileBuf;
    QJsonDocument doc;
    QFile saveFile(szFile);
    if (saveFile.open(QIODevice::ReadWrite)) {
        fileBuf = saveFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        saveFile.close();
    }
    if (!doc.isEmpty() && doc.isObject()) {
		setting = doc.object().toVariantMap();
    }
}

void Storage::setSettingToFile(QString szFile, QVariantMap &setting) {
    QJsonDocument doc(QJsonObject::fromVariantMap(setting));

    QFile saveFile(szFile);
    if (saveFile.open(QIODevice::ReadWrite)) {
        saveFile.resize(0);
        saveFile.write(doc.toJson(QJsonDocument::Compact));
        saveFile.close();
    }
}

bool Storage::LoadSoftwareCategory(QString szCategoryFile, QVariantList &mapCategory) {
    QByteArray fileBuf;
    QJsonObject jsObj;
    QJsonDocument doc;

    if (!QFile::exists(szCategoryFile)) {
        return false;
    }

    QFile loadFile(szCategoryFile);
    if (loadFile.open(QIODevice::ReadOnly)) {
        fileBuf = loadFile.readAll();
        QJsonParseError err;
        doc = QJsonDocument::fromJson(fileBuf,&err);
        loadFile.close();
    }
    else {
        return false;
    }
    if (doc.isEmpty() || !doc.isObject()) {
        return false;
    }
    jsObj = doc.object();
    if (!jsObj.contains("code") || !jsObj.value("code").isDouble() || !jsObj.contains("msg") || !jsObj.value("msg").isArray()) {
        return false;
    }
    if (jsObj.value("code").toInt() != 0) {
        return false;
    }
    mapCategory = jsObj.value("msg").toArray().toVariantList();

    return true;
}

bool Storage::LoadCategorySoftwareList(QString szCategoryListFile, QString szCategoryID, QMap<QString,QVariantList> &mapPackageByCategoryID) {
    QByteArray fileBuf;
    QJsonObject jsObj;
    QJsonDocument doc;

    QFile loadFile(szCategoryListFile);
    if (loadFile.open(QIODevice::ReadOnly)) {
        fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        loadFile.close();
    }
    else {
        return false;
    }
    if (doc.isEmpty() || !doc.isObject()) {
        return false;
    }
    jsObj = doc.object();
    if (!jsObj.contains("code") || !jsObj.value("code").isDouble() || !jsObj.contains("msg") || !jsObj.value("msg").isArray()) {
        return false;
    }
	if (jsObj.value("code").toInt() != 0) {
        return false;
    }
    mapPackageByCategoryID[szCategoryID] = jsObj.value("msg").toArray().toVariantList();
    return true;
}

bool Storage::LoadArrayOfSoftwareList(QString szSoftListFile, QVariantList &arrPackage) {
    QByteArray fileBuf;
    QJsonDocument doc;
    QJsonObject jsObj;

    QFile loadFile(szSoftListFile);
    if (loadFile.open(QIODevice::ReadOnly)) {
        fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        loadFile.close();
    }
    else {
        return false;
    }
    if (doc.isEmpty() || !doc.isObject()) {
        return false;
    }
    jsObj = doc.object();
    if (!jsObj.contains("code") || !jsObj.value("code").isDouble() || !jsObj.contains("msg") || !jsObj.value("msg").isArray()) {
        return false;
    }
	if (jsObj.value("code").toInt() != 0) {
        return false;
    }
    arrPackage = jsObj.value("msg").toArray().toVariantList();
    return true;
}

 void Storage::AddIntoTaskConf(QString szConfFile, LPDowningTaskObject task) {
     QVariantList contents;
	 bool exist = false;
     if (task==NULL || task->status==4) {
         return;
     }

     Storage::LoadTasksFromConfArray(szConfFile,contents);
     foreach(QVariant it,contents) {
         QVariantMap iterator = it.toMap();
         if (iterator.value("id").toString().compare(task->id)==0) {
             exist = true;
             break;
         }
     }
     if (!exist) {
         QVariantMap taskObject;
         taskObject.insert(QString("id"),QVariant::fromValue(task->id));
         taskObject.insert(QString("catid"),QVariant::fromValue(task->category));
         taskObject.insert(QString("name"),QVariant::fromValue(task->name));
         taskObject.insert(QString("largeIcon"),QVariant::fromValue(task->largeIcon));
         taskObject.insert(QString("brief"),QVariant::fromValue(task->brief));
         taskObject.insert(QString("size"),QVariant::fromValue(task->size));
         taskObject.insert(QString("percent"),QVariant::fromValue(task->percent));
         taskObject.insert(QString("speed"),QVariant::fromValue(0));
         taskObject.insert(QString("status"),QVariant::fromValue(task->status));
         taskObject.insert(QString("downloadUrl"),QVariant::fromValue(task->downloadUrl));
         taskObject.insert(QString("autoInstall"),QVariant::fromValue(task->autoInstall));
         taskObject.insert(QString("versionName"),QVariant::fromValue(task->versionName));
         taskObject.insert(QString("packageName"),QVariant::fromValue(task->packageName));
         taskObject.insert(QString("binaryImagePath"),QVariant::fromValue(task->binaryImagePath));

         contents.append(QVariant::fromValue(taskObject));
         Storage::SaveTasksFromConfArray(szConfFile, contents);
     }
 }

 void Storage::SaveTasks(QString szConfFile, mapDowningTaskObject & tasks) {
    QVariantList contents;

    for(mapDowningTaskObject::iterator it = tasks.begin();it!=tasks.end();it++) {
        if (it.value() != NULL && it.value()->status != 4) {
            LPDowningTaskObject task = it.value();
            QVariantMap taskObject;
            taskObject.insert(QString("id"),QVariant::fromValue(task->id));
            taskObject.insert(QString("catid"),QVariant::fromValue(task->category));
            taskObject.insert(QString("name"),QVariant::fromValue(task->name));
            taskObject.insert(QString("largeIcon"),QVariant::fromValue(task->largeIcon));
            taskObject.insert(QString("brief"),QVariant::fromValue(task->brief));
            taskObject.insert(QString("size"),QVariant::fromValue(task->size));
            taskObject.insert(QString("percent"),QVariant::fromValue(task->percent));
            taskObject.insert(QString("speed"),QVariant::fromValue(0));
            taskObject.insert(QString("status"),QVariant::fromValue(task->status));
            taskObject.insert(QString("downloadUrl"),QVariant::fromValue(task->downloadUrl));
            taskObject.insert(QString("autoInstall"),QVariant::fromValue(task->autoInstall));
            taskObject.insert(QString("versionName"),QVariant::fromValue(task->versionName));
            taskObject.insert(QString("packageName"),QVariant::fromValue(task->packageName));
            taskObject.insert(QString("binaryImagePath"),QVariant::fromValue(task->binaryImagePath));
            contents.append(taskObject);
        }
    }
    SaveTasksFromConfArray(szConfFile,contents);
}

void Storage::LoadTasks(QString szConfFile,mapDowningTaskObject & tasks) {
    QVariantList allItems;
    Storage::LoadTasksFromConfArray(szConfFile,allItems);
    foreach(QVariant it,allItems) {
        QVariantMap iterator = it.toMap();
        LPDowningTaskObject task = new DowningTaskObject();
        task->id = iterator.value("id").toString();
        task->category = iterator.value("catid").toString();
        task->name = iterator.value("name").toString();
        task->largeIcon = iterator.value("largeIcon").toString();
        task->brief = iterator.value("brief").toString();
        task->size = iterator.value("size").toLongLong();
        task->percent = iterator.value("percent").toFloat();
        task->speed = 0;
        task->status = iterator.value("status").toInt();
        task->downloadUrl = iterator.value("downloadUrl").toString();
        task->versionName = iterator.value("versionName").toString();
        task->packageName = iterator.value("packageName").toString();
        task->autoInstall = iterator.value("autoInstall").toBool();
        task->binaryImagePath = iterator.value("binaryImagePath").toString();
        tasks.insert(task->id,task);
    }
}

void Storage::LoadTasksFromConfArray(QString szConfFile,QVariantList &mapItems) {
    QJsonDocument doc;

    QFile loadFile(szConfFile);
    if (loadFile.open(QIODevice::ReadWrite)) {
        QByteArray fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        if (!doc.isEmpty() && doc.isArray()) {
            mapItems = doc.array().toVariantList();
        }
        else {
            loadFile.resize(0);
        }
        loadFile.close();
    }
}

void Storage::SaveTasksFromConfArray(QString szConfFile, QVariantList &mapItems) {
    QJsonDocument doc;
    QFile loadFile(szConfFile);
    if (loadFile.open(QIODevice::ReadWrite)) {
        QByteArray fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        loadFile.resize(0);
        doc.setArray(QJsonArray::fromVariantList(mapItems));
        loadFile.write(doc.toJson(QJsonDocument::Compact));
        loadFile.close();
    }
}

void Storage::AddIntoLauncherConf(QString szConfFile,QVariantMap &Item) {
    bool exist = false;
    QVariantList allItems;
    Storage::LoadLaunchFromConfigArray(szConfFile,allItems);
    foreach(QVariant it,allItems) {
        QVariantMap iterator = it.toMap();
        if (iterator.value("id").toString().compare(Item.value("id").toString())==0) {
            exist = true;
            break;
        }
    }
    if (!exist) {
		allItems.append(QVariant::fromValue(Item));
        Storage::SaveLaunchFromConfigArray(szConfFile,allItems);
    }
}

void Storage::LoadLaunchFromConfigArray(QString szConfFile,QVariantList &mapItems) {
    QJsonDocument doc;

    QFile loadFile(szConfFile);
    if (loadFile.open(QIODevice::ReadWrite)) {
        QByteArray fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        if (!doc.isEmpty() && doc.isArray()) {
            mapItems = doc.array().toVariantList();
        }
        else {
            loadFile.resize(0);
        }
        loadFile.close();
    }
}

void Storage::SaveLaunchFromConfigArray(QString szConfFile,QVariantList &mapItems) {
    QJsonDocument doc;

    QFile loadFile(szConfFile);
    if (loadFile.open(QIODevice::ReadWrite)) {
        QByteArray fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
		loadFile.resize(0);
        doc.setArray(QJsonArray::fromVariantList(mapItems));
        loadFile.write(doc.toJson(QJsonDocument::Compact));
        loadFile.close();
    }
}
