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

    // for launch list
    static bool AddTaskToConfArray(QString , QVariantMap &);
    static bool LoadTasksFromConfArray(QString , mapDowningTaskObject &);

signals:

public slots:
};

#endif // STORAGE_H
