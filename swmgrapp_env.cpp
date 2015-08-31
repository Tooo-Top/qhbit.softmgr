#include "swmgrapp.h"
#include "curl/curl.h"

const QString &SwmgrApp::GetEnvVar() {
    static QString refEnvVar("CommonProgramFiles");
    return refEnvVar;
}

const QString &SwmgrApp::GetCompanyName() {
    static const QString companyName("HurricaneTeam");
    return companyName;
}

const QString &SwmgrApp::GetSoftwareName() {
    static const QString softwareName("xbsoftMgr");
    return softwareName;
}

QString SwmgrApp::GetAppDataPath(QString szCompany) {
    QString szPath(""), szKey = SwmgrApp::GetEnvVar();
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.keys().contains(szKey, Qt::CaseInsensitive)) {
        szPath = env.value(szKey);
        szPath.append(QDir::separator());
        szPath.append(szCompany);
        szPath = QDir::toNativeSeparators(szPath);
    }
    return szPath;
}

QString SwmgrApp::GetProgramProfilePath(QString name) {
    QString szProgProfile = GetAppDataPath(SwmgrApp::GetCompanyName());
    szProgProfile.append(QDir::separator());
    szProgProfile.append(name);
    szProgProfile = QDir::toNativeSeparators(szProgProfile);
    return szProgProfile;
}

QString SwmgrApp::GetFilePathFromFile(QString szFile) {
    QFileInfo fieInfo(szFile);
    return QDir::toNativeSeparators(fieInfo.absolutePath());
}

QString SwmgrApp::GetCookieFile() {
    return GetProgramProfilePath(QString("xbsoftMgr")) + "\\xbsoftMgr.cookie";
}

QString SwmgrApp::GetUserLoginUrl() {
    return QString("http://ctr.datacld.com/api/user");
}

QString SwmgrApp::GetUserRegisteUrl() {
    return GetUserLoginUrl() + "/register";
}

void SwmgrApp::InitDir(QString szAppDir) {
    QDir dir;

    QStringList szItems = (QStringList()<<QString("UpdateDir")<<QString("Data")<<QString("Conf")<<QString("Temp") );
    ConfOperation::Root().setRootPath(szAppDir);// set root path
    ConfOperation::Root().initSubpath(szItems); // create sub path
}

BOOL SwmgrApp::InitCurl() {
    if (::curl_global_init(CURL_GLOBAL_WIN32) == CURLE_OK)
        m_bCurlStatus = TRUE;
    return m_bCurlStatus;
}

void SwmgrApp::DumpEnv() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    foreach(QString key, env.keys()) {
        qDebug() << key << "=" << env.value(key) << endl;
    }
}

QString SwmgrApp::getUserToken() {
    return _DataModel->getUserToken();
}

QString SwmgrApp::getSettingParameter(QString name, QString defaultValue) {
    return _DataModel->getSettingParameter(name, defaultValue);
}
