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

bool Storage::AddTaskToConfArray(QString szConfFile, QVariantMap &jsItem,QString type) {
    QByteArray fileBuf;
    QJsonDocument doc;
    QJsonArray jsArray;
    QJsonObject jsRootObject;
    bool exist = false;
    QJsonValue val;

    QFile loadFile(szConfFile);
    if (loadFile.open(QIODevice::ReadWrite)) {
        fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        if (!doc.isEmpty() && doc.isObject()) {
            jsRootObject = doc.object();
        }
        else {
            loadFile.resize(0);
        }

        if (jsRootObject.isEmpty()) {
            jsRootObject.insert("launchs",QJsonValue(QJsonArray::fromVariantList(QVariantList())));
			jsRootObject.insert("tasks", QJsonValue(QJsonArray::fromVariantList(QVariantList())));
        }
        if (!jsRootObject.contains("launchs")) {
			jsRootObject.insert("launchs", QJsonValue(QJsonArray::fromVariantList(QVariantList())));
        }
        if (!jsRootObject.contains("tasks")) {
			jsRootObject.insert("tasks", QJsonValue(QJsonArray::fromVariantList(QVariantList())));
        }

        if (type.compare("launchs")==0) {
            jsArray = jsRootObject.value("launchs").toArray();
            exist = false;
            foreach(val,jsArray) {
                if (val.toObject().value("id").toString().compare(jsItem.value("id").toString())==0) {
                    exist = true;
                }
            }
            if (!exist) {
                jsArray.append(QJsonObject::fromVariantMap(jsItem));
                jsRootObject.remove("launchs");
                jsRootObject.insert("launchs",QJsonValue(jsArray));
            }
        }
        else if (type.compare("tasks")==0) {
            jsArray = jsRootObject.value("tasks").toArray();
            exist = false;
            foreach(val,jsArray) {
                if (val.toObject().value("id").toString().compare(jsItem.value("id").toString())==0) {
                    exist = true;
                }
            }
            if (!exist) {
                jsArray.append(QJsonObject::fromVariantMap(jsItem));
                jsRootObject.remove("tasks");
                jsRootObject.insert("tasks",QJsonValue(jsArray));
            }
        }
        loadFile.resize(0);
        loadFile.write(doc.toJson(QJsonDocument::Compact));
        loadFile.close();
        return true;
    }
    else {
        return false;
    }
}

bool Storage::LoadTasksFromConfArray(QString szConfFile, mapDowningTaskObject & mapTaskObject, QString type) {
    QJsonDocument doc;
    QJsonObject jsRootObject;
    QJsonValue it;
    QJsonArray jsLaunchArray,jsTaskStatusArray;
    LPDowningTaskObject taskObject = NULL;

    QFile loadFile(szConfFile);
    if (loadFile.open(QIODevice::ReadWrite)) {
        QByteArray fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        if (!doc.isEmpty() && doc.isObject()) {
            jsRootObject = doc.object();
        }
        else {
            loadFile.resize(0);
        }

        if (jsRootObject.isEmpty()) {
            loadFile.resize(0);
            loadFile.close();
            return true;
        }

        if (type.compare("launchs")==0) {
            if (jsRootObject.contains("launchs") && jsRootObject.value("launchs").isArray()) {
                jsLaunchArray = jsRootObject.value("launchs").toArray();
            }
            else {
                loadFile.resize(0);
            }
        }
        else if (type.compare("tasks")==0) {
            if (jsRootObject.contains("tasks") && jsRootObject.value("tasks").isArray()) {
				jsTaskStatusArray = jsRootObject.value("tasks").toArray();
            }
        }
        else {
            loadFile.resize(0);
        }
        loadFile.close();

        foreach(it, jsTaskStatusArray) {
            QJsonObject jsObject = it.toObject();
            if (jsObject.value("id").toString().isEmpty()){continue;}

            mapDowningTaskObject::iterator itTask = mapTaskObject.find(jsObject.value("id").toString());
            if (itTask!=mapTaskObject.end()) {
                // change status
                taskObject = itTask.value();
                if (!taskObject) {
					mapTaskObject.erase(itTask);
                }
                else if (taskObject->status==2 || taskObject->status==3 || taskObject->status==4 || taskObject->status==6 ) {
                    taskObject->status = 0;
                }
            }
            else {
                taskObject = new DowningTaskObject();
                taskObject->id = jsObject.value("id").toString();
                taskObject->category = jsObject.value("category").toString();
                taskObject->name = jsObject.value("name").toString();
                taskObject->largeIcon= jsObject.value("largeIcon").toString();
                taskObject->brief= jsObject.value("brief").toString();
                taskObject->size= jsObject.value("size").toInt();
                taskObject->percent=jsObject.value("percent").toDouble();
                taskObject->speed  = 0;
                taskObject->status = jsObject.value("status").toInt();
                taskObject->downloadUrl = jsObject.value("downloadUrl").toString();
                taskObject->versionName = jsObject.value("versionName").toString();
                taskObject->packageName = jsObject.value("packageName").toString();

                taskObject->autoInstall = jsObject.value("autoInstall").toBool();
                taskObject->hTaskHandle = NULL;
                taskObject->launchName  = taskObject->name;

                mapTaskObject.insert(taskObject->id, taskObject);
            }
        }
        foreach(it, jsLaunchArray) {
            QJsonObject jsObject = it.toObject();
            if (jsObject.value("id").toString().isEmpty()){continue;}

            mapDowningTaskObject::iterator itTask = mapTaskObject.find(jsObject.value("id").toString());
            if (itTask!=mapTaskObject.end()) {
                // change status
                taskObject = itTask.value();
                if (!taskObject) {
					mapTaskObject.erase(itTask);
                }
            }
            else {
                taskObject = new DowningTaskObject();
                taskObject->id = jsObject.value("id").toString();
                taskObject->category = jsObject.value("catid").toString();
				taskObject->autoInstall = jsObject.value("autoInstall").toBool();
                mapTaskObject.insert(taskObject->id, taskObject);
            }
        }
        return true;
    }
    else {
        return false;
    }
}
