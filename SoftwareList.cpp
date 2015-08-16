#include "SoftwareList.h"
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

QString categoryItems[nCategoryItemCount] = {
	"id",
	"name",
	"alias",
	"total"
};

QString packageItems[nPackageItemCount] = {
	"id",
	"packageName",
	"windowsVersion",
	"arch",
	"name",
	"category",
	"description",
	"developer",
	"iconUrl",
	"largeIcon",

	"screenshotsUrl",
	"incomeShare",
	"rating",
	"versionName",
	"versionCode",
	"priceInfo",
	"tag",
	"downloadUrl",
	"hash",
	"size",

	"createTime",
	"updateTime",
	"signature",
	"updateInfo",
	"language",
	"brief",
	"isAd",
	"status"
};

bool SoftwareList::LoadSoftwareCategory(QString szCategoryFile, QJsonArray &mapCategory) {
	QByteArray fileBuf;
	QJsonArray jsArray;
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
	mapCategory = jsObj.value("msg").toArray();

    return true;
}

bool SoftwareList::LoadCategorySoftwareList(QString szCategoryListFile, QString szCategoryID, QMap<QString, QJsonArray> &mapPackageByCategoryID) {
	QByteArray fileBuf;
	QJsonArray jsArray;
	QJsonDocument doc;
	QJsonObject jsObj;

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
	mapPackageByCategoryID[szCategoryID] = jsObj.value("msg").toArray();
	return true;
}

bool SoftwareList::LoadArrayOfSoftwareList(QString szSoftListFile, QJsonArray &arrPackage) {
	QByteArray fileBuf;
	QJsonArray jsArray;
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
	arrPackage = jsObj.value("msg").toArray();
	return true;
}

bool SoftwareList::AddItemToConfArray(QString szConfFile, QJsonObject &jsItem) {
	QByteArray fileBuf;
	QJsonDocument doc;
	QJsonArray jsArray;

	QFile loadFile(szConfFile);
	if (loadFile.open(QIODevice::ReadWrite)) {
		fileBuf = loadFile.readAll();
		doc = QJsonDocument::fromJson(fileBuf);
		if (!doc.isEmpty() && doc.isArray()) {
			jsArray = doc.array();
		}
		else {
			loadFile.resize(0);
		}
		// check save to file
		if (!jsArray.contains(QJsonValue(jsItem))) {
			jsArray.append(jsItem);
			doc.setArray(jsArray);

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

bool SoftwareList::LoadArrayFromConf(QString szConfFile, mapDowningTaskObject & mapTaskObject) {
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
					taskObject->status = 2;
					taskObject->hTaskHandle = NULL;

					mapTaskObject.insert(taskObject->id, taskObject);

					qDebug() << "add task :" << jsObject.value("id").toString() << "," << jsObject.value("catid").toString() << "," << jsObject.value("launchName").toString();
				}
				else {
					qDebug() << "repeat task :" << jsObject.value("id").toString() << "," << jsObject.value("catid").toString() << "," << jsObject.value("launchName").toString();
				}
			}
		}
		return true;
	}
	else {
		return false;
	}
}

void SoftwareList::getSettingFromFile(QString szFile, QJsonObject &setting) {
	QByteArray fileBuf;
	QJsonDocument doc;
	QFile saveFile(szFile);
	if (saveFile.open(QIODevice::ReadWrite)) {
		fileBuf = saveFile.readAll();
		doc = QJsonDocument::fromJson(fileBuf);
		saveFile.close();
	}
	if (doc.isEmpty() || !doc.isObject()) {
		return;
	}
	setting = doc.object();
}
void SoftwareList::setSettingToFile(QString szFile, QJsonObject &setting) {
	QJsonDocument doc(setting);

	QFile saveFile(szFile);
	if (saveFile.open(QIODevice::ReadWrite)) {
		saveFile.resize(0);
		saveFile.write(doc.toJson(QJsonDocument::Compact));
		saveFile.close();
	}
}
