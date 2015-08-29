#ifndef STORAGE_H
#define STORAGE_H

#include <QObject>
#include "PackageRunner.h"


class Storage : public QObject
{
    Q_OBJECT
public:
    explicit Storage(QObject *parent = 0);
public:
    // for programe setting
    static void getSettingFromFile(QString, QVariantMap&);
    static void setSettingToFile(QString, QVariantMap&);

    static bool LoadSoftwareCategory(QString, QVariantList &);

    // for package list
    static bool LoadCategorySoftwareList(QString, QString, QMap<QString,QVariantList> &);
    static bool LoadArrayOfSoftwareList(QString, QVariantList &);

    // for task status
//    static bool AddTaskToConfArray(QString , QVariantMap & , QString);
//    static void LoadTasksFromConfArray(QString , mapDowningTaskObject & , QString);

    static void AddIntoTaskConf(QString , LPDowningTaskObject );
    static void SaveTasks(QString , mapDowningTaskObject & );
    static void LoadTasks(QString , mapDowningTaskObject & );
    static void SaveTasksFromConfArray(QString , QVariantList &);
    static void LoadTasksFromConfArray(QString , QVariantList &);

    // for launch list
    static void AddIntoLauncherConf(QString,QVariantMap &);
    static void LoadLaunchFromConfigArray(QString,QVariantList &);
    static void SaveLaunchFromConfigArray(QString,QVariantList &);
    //

signals:

public slots:
};

#endif // STORAGE_H
