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

bool Storage::AddItemToConfArray(QString szConfFile, QVariantMap &jsItem) {
    QByteArray fileBuf;
    QJsonDocument doc;
    QVariantList jsArray;

    QFile loadFile(szConfFile);
    if (loadFile.open(QIODevice::ReadWrite)) {
        fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        if (!doc.isEmpty() && doc.isArray()) {
            jsArray = doc.array().toVariantList();
        }
        else {
            loadFile.resize(0);
        }
        // check save to file
        if (!jsArray.contains(jsItem)) {
            jsArray.append(jsItem);
            doc.setArray(QJsonArray::fromVariantList(jsArray));

            loadFile.resize(0);
            loadFile.write(doc.toJson(QJsonDocument::Compact));
        }
        loadFile.close();
        return true;
    }
    else {
        return false;
    }
}

bool Storage::LoadItemsFromConfArray(QString szConfFile, mapDowningTaskObject & mapTaskObject) {
    QJsonDocument doc;
    QJsonArray jsArray;

    QFile loadFile(szConfFile);
    if (loadFile.open(QIODevice::ReadWrite)) {
        QByteArray fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        if (!doc.isEmpty() && doc.isArray()) {
            jsArray = doc.array();
        }
        else {
            loadFile.resize(0);
        }
        loadFile.close();

        foreach(QJsonValue it, jsArray) {
            QJsonObject jsObject = it.toObject();
            if (jsObject.contains("id") && jsObject.value("id").isString() &&
                jsObject.contains("launchName") && jsObject.value("launchName").isString()
                ) {
                if (mapTaskObject.find(jsObject.value("id").toString()) == mapTaskObject.end()) {
                    LPDowningTaskObject taskObject = new DowningTaskObject();
                    taskObject->id = jsObject.value("id").toString();
                    taskObject->category = jsObject.value("catid").toString();
                    taskObject->launchName = jsObject.value("launchName").toString();
                    taskObject->autoInstall = jsObject.value("autoInstall").toBool();
                    taskObject->status = 2;
                    taskObject->hTaskHandle = NULL;

                    mapTaskObject.insert(taskObject->id, taskObject);

                    qDebug() << "add task :" << jsObject.value("id").toString() << ","
                        << jsObject.value("catid").toString() << ","
                        << jsObject.value("launchName").toString() << ","
                        << jsObject.value("autoInstall").toBool();
                        return true;
                }
                else {
                    qDebug() << "repeat task :" << jsObject.value("id").toString() << ","
                        << jsObject.value("catid").toString() << ","
                        << jsObject.value("launchName").toString() << ","
                        << jsObject.value("autoInstall").toBool();
                }
            }
        }
        return true;
    }
    else {
        return false;
    }
}
