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
BOOL SwmgrApp::InitMiniXL() {
    _pWapper = LoadDll();
    if (!_pWapper) {
        return FALSE;
    }
    if (!_pWapper->Init()) {
        UnloadDll(&_pWapper);
        return FALSE;
    }
    return TRUE;
}

void SwmgrApp::DumpEnv() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    foreach(QString key, env.keys()) {
        qDebug() << key << "=" << env.value(key) << endl;
    }
}

DownWrapper* SwmgrApp::LoadDll()
{
    WCHAR szDllpath[512] = { 0 };
    QString szLoadPath = SwmgrApp::GetProgramProfilePath(QString("xbSpeed"));
    szLoadPath.append( QDir::separator() + QString("xldl.dll") );
    szLoadPath = QDir::toNativeSeparators(szLoadPath);

    StrCpyW(szDllpath, szLoadPath.toStdWString().data());
    DownWrapper* pWapper = NULL;
    try{
        pWapper = new DownWrapper(szDllpath);
    }
    catch (wchar_t e[]) {
        pWapper = NULL;
        qDebug() << "*****************:"<<QString::fromWCharArray(e);
    }
    return pWapper;
}

void SwmgrApp::UnloadDll(DownWrapper** Wapper){
    if (!Wapper) {
        return;
    }
    if ((*Wapper) != NULL) {
        (*Wapper)->UnInit();
        delete (*Wapper);
        (*Wapper) = NULL;
    }
}
//
//#include <comdef.h>
//#include <comutil.h>
//#include <atlbase.h>
//#include <atlcomcli.h>
//#include <Shobjidl.h>
//
//BOOL SwmgrApp::CreateShellLink(QString szTargetExec,QString szID,QString szCategory,QString szLnkName,QString szIconName,QString szDescription) {
//    CComPtr<IShellLink>   pShortCutLink;    //IShellLink对象指针
//    CComPtr<IPersistFile> ppf;		        //IPersisFile对象指针
//
//    QString szLnkPath,szWorkingDirectory,szArguments;
//    QString szKey("USERPROFILE");
//
//    QFileInfo fieInfo(szTargetExec);
//    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
//    // Initialize var
//    if (env.keys().contains(szKey, Qt::CaseInsensitive)) {
//        szLnkPath = env.value(szKey);
//        szLnkPath.append(QDir::separator());
//        szLnkPath.append("Desktop");
//        szLnkPath.append(QDir::separator());
//        szLnkPath.append(szLnkName);
//        szLnkPath = QDir::toNativeSeparators(szLnkPath);
//    }
//    else {
//        return FALSE;
//    }
//    if (QFile::exists(szLnkPath)) {
//        return TRUE;
//    }
//    szWorkingDirectory = QDir::toNativeSeparators(fieInfo.absolutePath());
//
//    szArguments = QString("install id=%1 catid=%2 %3").arg(szID).arg(szCategory).arg(szLnkName);
//
//    // Create shelllink
//    CoInitialize(NULL);
//    pShortCutLink.CoCreateInstance(CLSID_ShellLink, NULL);
//    // Set shelllink parameters
//    pShortCutLink->SetPath(szTargetExec.toStdWString().data());
//    pShortCutLink->SetArguments(szArguments.toStdWString().data());
//    pShortCutLink->SetWorkingDirectory(szWorkingDirectory.toStdWString().data());
//    pShortCutLink->SetIconLocation(szIconName.toStdWString().data(), 0);
//    pShortCutLink->SetDescription(szDescription.toStdWString().data());
//    pShortCutLink->SetShowCmd(SW_SHOW);
//    // Save shelllink
//    pShortCutLink->QueryInterface(&ppf);
//    ppf->Save(szLnkPath.toStdWString().data(), FALSE);
//    CoUninitialize();
//    return TRUE;
//}
