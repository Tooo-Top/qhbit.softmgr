#ifndef SOFTWARELIST_H
#define SOFTWARELIST_H
#include <QMap>
#include <QVector>
#include <QJsonObject>
#include "DownWrapper.h"

//--------json soft list parameter use these  **begin
const int nCategoryItemCount = 4;
const int nPackageItemCount = 28;
/*
extern QString categoryItems[nCategoryItemCount];
extern QString packageItems[nPackageItemCount];
//--------json soft list parameter use these  **end

typedef QMap<QString, QString> CommonItem;
typedef QMap<QString, CommonItem> mapSoftwareCategory;

typedef QVector<CommonItem> lstSoftwarePackage;
typedef QMap<QString, lstSoftwarePackage> mapSoftwarePackages;
*/
/*
* status follow in configure file
*     0 : just add
*     1 : started
*     2 : pause
*     3 : finish
*     4 : download err
*     5 : delete
*     6 : execute err
*/

typedef struct __DowningTaskObject {
	QString id;
	QString name;
	QString category;
	QString launchName;
	DownTaskParam downTaskparam;
	int status; //0:added,1:start,2:pause,3:stop,4:del,5:err,6:param err
    float percent;
	HANDLE hTaskHandle;
}DowningTaskObject, *LPDowningTaskObject;

typedef QMap<QString, LPDowningTaskObject> mapDowningTaskObject;

class SoftwareList {
protected:
    SoftwareList(){};
public:
    // for category list
	static bool LoadSoftwareCategory(QString, QJsonArray &);

    // for package list
	static bool LoadCategorySoftwareList(QString, QString, QMap<QString, QJsonArray>&);
	static bool LoadArrayOfSoftwareList(QString, QJsonArray &);

	// for launch list
	static bool AddItemToConfArray(QString , QJsonObject &);
	static bool LoadArrayFromConf(QString , mapDowningTaskObject &);

	// for programe setting
	static void getSettingFromFile(QString, QJsonObject&);
	static void setSettingToFile(QString, QJsonObject&);
};

#endif // SOFTWARELIST_H
