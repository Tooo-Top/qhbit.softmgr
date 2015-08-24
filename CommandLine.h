#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

class CommandLine : public QObject
{
    Q_OBJECT
public:
    explicit CommandLine(QObject *parent = 0);
public:
    int parseCommandLine(QStringList commandLine);
    QVariantMap encodeToVariantMap();
public:
    bool launchMode();
    QString launchID();
    QString launchCatID();
    QString launchName();
protected:
    bool bMode;
    QString _id;
    QString _catid;
    QString _launchName;

signals:

public slots:
};

#endif // COMMANDLINE_H
