#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>
#include <QMap>


const int userinfoItemCount = 13;
extern QString userinfoItem[userinfoItemCount];

class UserInfo : public QObject
{
    Q_OBJECT
public:
    explicit UserInfo(QObject *parent = 0);
protected:
	QString init;
	QString username;
	QString password;
	QString usertoken;
	QMap<QString, QString> userPrivateInfo;
protected:
	static QByteArray cryptPassword(QString szPassword);
public:
	void serializeUserInfo(bool bSerialize=false);
    QJsonObject toJsonObject();

protected:
    QString postMethod(std::string url, std::string cookieFile, std::string post);
	static size_t LoginCallback(char *buffer, size_t size, size_t nitems, void *outstream);
signals:

public slots :
	void UserLogin(QString szUserName, QString szPassword);
	void RegistUser(QString,QString,QString);
};

#endif // USERINFO_H
