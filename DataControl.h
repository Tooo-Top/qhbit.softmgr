#ifndef DATACONTROL_H
#define DATACONTROL_H

#include "ConfOperation.h"
#include "SoftwareList.h"

class DataControl
{
public:
    DataControl();

protected:
	// -----------------------
	mapSoftwareCategory _softCategory;  //category list
	mapSoftwarePackages _softPackages;  //package list by category
	lstSoftwarePackage  _softTopPackages;
	lstSoftwarePackage  _softHotPackages;
	// -----------------------
	
public:
	mapSoftwareCategory &getSoftCategory();
	mapSoftwarePackages &getSoftPackages();
	lstSoftwarePackage  &getTopPackages();
	lstSoftwarePackage  &getHotPackages();
public:
	bool initSoftCategory();
	bool initSoftPackages();
	bool initTopPackages();
	bool initHotPackages();

	bool initAll();
};

#endif // DATACONTROL_H
