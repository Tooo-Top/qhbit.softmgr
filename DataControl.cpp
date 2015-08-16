#include "DataControl.h"

DataControl::DataControl()
{

}

QJsonArray &DataControl::getSoftCategory() {
	return _softCategory;  //category list
}

QMap<QString, QJsonArray> &DataControl::getSoftPackages() {
	return _softPackages;  //package list by category
}

QJsonArray &DataControl::getTopPackages() {
	return  _softTopPackages;
}

QJsonArray  &DataControl::getHotPackages() {
	return  _softHotPackages;
}

bool DataControl::initSoftCategory() {
	return SoftwareList::LoadSoftwareCategory(ConfOperation::Root().getSubpathFile("Data", "SoftwareCategoryAll.list"), _softCategory);
}

bool DataControl::initSoftPackages() {
	foreach(QJsonValue category, _softCategory) {
		QString szSubCategroySoftlist = ConfOperation::Root().getSubpathFile("Data", QString("SoftwareCategory") + category.toObject().value("id").toString() + ".list");
		SoftwareList::LoadCategorySoftwareList(szSubCategroySoftlist, category.toObject().value("id").toString(), _softPackages);
	}
	return true;
}

bool DataControl::initTopPackages() {
	return SoftwareList::LoadArrayOfSoftwareList(ConfOperation::Root().getSubpathFile("Data", "SoftwareCategoryTOP.list"), _softTopPackages);
}

bool DataControl::initHotPackages() {
	return SoftwareList::LoadArrayOfSoftwareList(ConfOperation::Root().getSubpathFile("Data", "SoftwareCategoryHOT.list"), _softHotPackages);
}

bool DataControl::initAll() {
	initSoftCategory();
	initSoftPackages();
	initTopPackages();
	initHotPackages();
	return true;
}
