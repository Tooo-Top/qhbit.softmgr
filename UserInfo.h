#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>

#include <QMap>
#include <QVariantMap>
#include <QFileSystemWatcher>

const int userinfoItemCount = 13;
extern QString userinfoItem[userinfoItemCount];

class UserInfo : public QObject
{
    Q_OBJECT
public:
    explicit UserInfo(QObject *parent = 0);
protected:
    // property "init":
    // 0xff:all informatin uninitialize;
    // 0:only has 'username';
    // 1:have 'username,password',but not login;
    // 2:have 'username,password' already login
    QString init;
	QString username;
	QString password;
	QString usertoken;
	QMap<QString, QString> userPrivateInfo;
    QFileSystemWatcher* _userDatWatcher;
protected:
	static QByteArray cryptPassword(QString szPassword);
public:
	void serializeUserInfo(bool bSerialize=false);
    QJsonObject toJsonObject();
protected:
    void clean();

    QString postMethod(std::string url, std::string cookieFile, std::string post);
	static size_t LoginCallback(char *buffer, size_t size, size_t nitems, void *outstream);

signals:
    void signalRegisteUser(QVariantMap userinfo);
    void signalLoginUser(QVariantMap userinfo);
    void signalModifyUserInfo(QVariantMap userInfo);
public slots :
	void UserLogin(QString szUserName, QString szPassword);
    void RegistUser(QString szUserName,QString szPassword,QString szEmail);
    void ModifyUserInfo(QVariantMap userInfo);
    void QueryUserInfo();

    void StartWatcher();
    void StopWatcher();
protected slots:
    void ChangedDat(QString path);
};

#endif // USERINFO_H
