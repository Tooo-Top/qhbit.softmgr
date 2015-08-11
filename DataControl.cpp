#include "DataControl.h"

DataControl::DataControl()
{

}

mapSoftwareCategory &DataControl::getSoftCategory() {
	return _softCategory;  //category list
}

mapSoftwarePackages &DataControl::getSoftPackages() {
	return _softPackages;  //package list by category
}

lstSoftwarePackage  &DataControl::getTopPackages() {
	return  _softTopPackages;
}

lstSoftwarePackage  &DataControl::getHotPackages() {
	return  _softHotPackages;
}

bool DataControl::initSoftCategory() {
	return SoftwareList::LoadSoftwareCategory(ConfOperation::Root().getSubpathFile("Data", "SoftwareCategoryAll.list"), _softCategory);
}

bool DataControl::initSoftPackages() {
	foreach(CommonItem category, _softCategory) {
		QString szSubCategroySoftlist = ConfOperation::Root().getSubpathFile("Data", QString("SoftwareCategory") + category["id"] + ".list");
		SoftwareList::LoadCategorySoftwareList(szSubCategroySoftlist, category["id"], _softPackages);
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
