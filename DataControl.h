#ifndef DATACONTROL_H
#define DATACONTROL_H
#include <QJsonArray>
#include "ConfOperation.h"
#include "SoftwareList.h"

class DataControl
{
public:
    DataControl();

protected:
	// -----------------------
	QJsonArray _softCategory;  //category list
	QMap<QString,QJsonArray> _softPackages;  //package list by category
	QJsonArray  _softTopPackages;
	QJsonArray  _softHotPackages;
	// -----------------------
	
public:
	QJsonArray &getSoftCategory();
	QMap<QString, QJsonArray> &getSoftPackages();
	QJsonArray  &getTopPackages();
	QJsonArray  &getHotPackages();
public:
	bool initSoftCategory();
	bool initSoftPackages();
	bool initTopPackages();
	bool initHotPackages();

	bool initAll();
};

#endif // DATACONTROL_H
