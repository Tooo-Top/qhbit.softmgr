#include "swmgrapp.h"
#include <QApplication>

#include <QMenu>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include "curl/curl.h"
#include "global.h"
#include "OSSystemWrapper.h"

SwmgrApp::SwmgrApp(QObject *parent) : QObject(parent)
{
	m_bCurlStatus = FALSE;
}
SwmgrApp::~SwmgrApp() {
	if (m_bCurlStatus) {
		::curl_global_cleanup();
	}
}

SwmgrApp *SwmgrApp::Instance() {
    static SwmgrApp *_Instance = new SwmgrApp(qApp);
    return _Instance;
}

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

BOOL SwmgrApp::InitAppEnv() {
    //DumpEnv();
	InitDir(SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()));
	LoadSettingProfile();
	if (!InitCurl()) {
		return FALSE;
	}
	if (!InitMiniXL()) {
		return FALSE;
	}
	// ----------
	InitObjects();
	InitIcons();
	InitMenuActions();
	InitSlots();
	InitTray();
	InitWnd();
	_DataModel.initAll();
	_PendingTasks.initTasks();
	InitNoticeServer();
	StartPoll();
//	_user.RegistUser("xiehc", "password", "xiehechong@sina.com");
	_user.UserLogin("xiehc", "password");
//    _window->load(QUrl::fromUserInput("D:/workspace/trunk/lewang/Index.html"));
//    _window->show();

	return TRUE;
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

void SwmgrApp::InitObjects() {
	appTrayIcon = new QIcon();

	trayIconMenu = new QMenu(NULL);
	fullAction = new QAction(QString("Full"), this);
	miniAction = new QAction(QString("Min"), this);
	quitAction = new QAction(QString("Close"), this);
	traySystem = new QSystemTrayIcon(this);
	srvLaunchInst = new QLocalServer(this);
	pollDownloadTaskObject = new QTimer(this);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);
    QWebSettings::globalSettings()->setMaximumPagesInCache(0);
    QWebSettings::globalSettings()->setObjectCacheCapacities(0, 0, 0);
    QWebSettings::globalSettings()->setOfflineStorageDefaultQuota(0);
    QWebSettings::globalSettings()->setOfflineWebApplicationCacheQuota(0);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);

	wndMain = new MainWnd();
	_webPage = new QWebPage();
	wndMain->setPage(_webPage);
}

void SwmgrApp::InitIcons() {
	appTrayIcon->addFile(":/xbmgr.ico");
}

void SwmgrApp::InitMenuActions() {
	trayIconMenu->addAction(fullAction);
	trayIconMenu->addAction(miniAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(quitAction);
}

void SwmgrApp::InitSlots() {
	QObject::connect(quitAction, SIGNAL(triggered(bool)), this, SLOT(appquit()));
	QObject::connect(fullAction, SIGNAL(triggered(bool)), this, SLOT(showFullWnd()));
	QObject::connect(miniAction, SIGNAL(triggered(bool)), this, SLOT(showMiniWnd()));
	QObject::connect(srvLaunchInst, SIGNAL(newConnection()), this, SLOT(launchInstall()));
	QObject::connect(this, SIGNAL(sigInstaller(QJsonObject)), SLOT(addInstaller(QJsonObject)));
	QObject::connect(pollDownloadTaskObject, SIGNAL(timeout()), this,SLOT(downloadPoll()));
}

void SwmgrApp::InitTray() {
	traySystem->setIcon(*appTrayIcon);
	traySystem->setContextMenu(trayIconMenu);
	traySystem->show();
}

void SwmgrApp::InitWnd() {
	//wndFull = new Widget(NULL);
    wndMain->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
    wndMain->setMouseTracking(true);
    wndMain->setFixedSize(963, 595);
//    wndMain->setUrl(QUrl::fromUserInput("D:/workspace/trunk/lewang/Index.html"));
//    wndMain->setUrl(QUrl::fromUserInput(GLOBAL::_DY_DIR_RUNNERSELF +"/lewang/Index.html"));
//	QObject::connect(wndMain->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(initWebViewHost()));
    _webPage->mainFrame()->load(QUrl::fromUserInput(GLOBAL::_DY_DIR_RUNNERSELF + "/lewang/Index.html"));
    //_webPage->mainFrame()->load(QUrl::fromUserInput("qrc:/index.html"));
    _webPage->triggerAction(QWebPage::Reload,false);
	QObject::connect(_webPage->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(initWebViewHost()));
    QObject::connect(_webPage->mainFrame(), SIGNAL(loadFinished(bool)), this, SLOT(docLoadFinish(bool)));
    wndMain->show();
}

void SwmgrApp::InitNoticeServer() {
	if (!srvLaunchInst->isListening()) {
		srvLaunchInst->listen("launch_pipe");
	}
}

void SwmgrApp::StartPoll() {
	pollDownloadTaskObject->start(500);
}

void SwmgrApp::UninitEnv() {

}

void SwmgrApp::DumpEnv() {
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	foreach(QString key, env.keys()) {
		qDebug() << key << "=" << env.value(key) << endl;
	}
}

void SwmgrApp::LoadSettingProfile() {
	QString szFile = ConfOperation::Root().getSubpathFile("Conf", "swgmgr.conf");
	SoftwareList::getSettingFromFile(szFile, _setting);
}

void SwmgrApp::SaveSettingProfile() {
	QString szFile = ConfOperation::Root().getSubpathFile("Conf", "swgmgr.conf");
	SoftwareList::setSettingToFile(szFile, _setting);
}

QString SwmgrApp::getSettingParameter(QString name, QString defaultValue) {
	if (_setting.isEmpty() || !_setting.contains(name)) {
		_setting.insert(name, defaultValue);
		SaveSettingProfile();
		return defaultValue;
	}
	return _setting.value(name).toString(defaultValue);
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

void SwmgrApp::appquit() {
    wndMain->close();
	_webPage->deleteLater();
	wndMain->deleteLater();
    QWebSettings::globalSettings()->clearMemoryCaches();
    qApp->quit();
//	QCoreApplication::exit(0);
}

void SwmgrApp::showFullWnd() {
    if (wndMain->isVisible())
        wndMain->hide();
    else {
        wndMain->show();
    }
}

void SwmgrApp::showMiniWnd() {
//    if (wndMain->isVisible())
//        wndMain->hide();
	// show mini widget
}

void SwmgrApp::initWebViewHost() {
    wndMain->page()->mainFrame()->addToJavaScriptWindowObject("DYBC",this);
}

void SwmgrApp::docLoadFinish(bool ok) {
    if (ok) {
//        wndMain->setFixedSize(_webPage->mainFrame()->contentsSize());
//        wndMain->page()->mainFrame()->evaluateJavaScript("document.documentElement.style.webkitUserSelect='none';");
	}
}

void SwmgrApp::monitorProf() {
	;//
}

void SwmgrApp::NoticeMain(QObject *parent, QJsonObject &jsItem) {
	QLocalSocket *sock = new QLocalSocket(parent);
	sock->connectToServer("launch_pipe");
	int nCount = 6;
	while (nCount > 0) {
		if (sock->waitForConnected(500)) {
			sock->write(QJsonDocument(jsItem).toJson(QJsonDocument::Compact));
			break;
		}
		else {
			nCount--;
		}
	}
	sock->close();
	sock->deleteLater();
}

void SwmgrApp::launchInstall() {
	QByteArray byteBuf;
	QJsonObject jsAddInstallObject;
	QJsonDocument jsDoc;
	QLocalSocket *localSock = srvLaunchInst->nextPendingConnection();

	int nCount = 5;

	do {
		if (localSock->waitForReadyRead(100)) {
			byteBuf.append(localSock->readAll());
			jsDoc = QJsonDocument::fromJson(byteBuf);
			if (jsDoc.isObject()) {
				break;
			}
			else {
				nCount--;
				continue;
			}
		}
		nCount--;
	} while (nCount > 0);
	
	localSock->close();
	localSock->deleteLater();
	if (jsDoc.isEmpty() || !jsDoc.isObject()) {
		return;
	}
	jsAddInstallObject = jsDoc.object();
    emit sigInstaller(jsAddInstallObject);
}

#include <comdef.h>
#include <comutil.h>
#include <atlbase.h>
#include <atlcomcli.h>
#include <Shobjidl.h>

BOOL SwmgrApp::CreateShellLink(QString szTargetExec,QString szID,QString szCategory,QString szLnkName,QString szIconName,QString szDescription) {
    CComPtr<IShellLink>   pShortCutLink;    //IShellLink对象指针
	CComPtr<IPersistFile> ppf;		        //IPersisFile对象指针

	QString szLnkPath,szWorkingDirectory,szArguments;
	QString szKey("USERPROFILE");

	QFileInfo fieInfo(szTargetExec);
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	// Initialize var
	if (env.keys().contains(szKey, Qt::CaseInsensitive)) {
		szLnkPath = env.value(szKey);
		szLnkPath.append(QDir::separator());
		szLnkPath.append("Desktop");
		szLnkPath.append(QDir::separator());
		szLnkPath.append(szLnkName);
		szLnkPath = QDir::toNativeSeparators(szLnkPath);
	}
	else {
		return FALSE;
	}
	if (QFile::exists(szLnkPath)) {
		return TRUE;
	}
	szWorkingDirectory = QDir::toNativeSeparators(fieInfo.absolutePath());

	szArguments = QString("install id=%1 catid=%2 %3").arg(szID).arg(szCategory).arg(szLnkName);

	// Create shelllink
	CoInitialize(NULL);
	pShortCutLink.CoCreateInstance(CLSID_ShellLink, NULL);
	// Set shelllink parameters
	pShortCutLink->SetPath(szTargetExec.toStdWString().data());
	pShortCutLink->SetArguments(szArguments.toStdWString().data());
	pShortCutLink->SetWorkingDirectory(szWorkingDirectory.toStdWString().data());
	pShortCutLink->SetIconLocation(szIconName.toStdWString().data(), 0);
	pShortCutLink->SetDescription(szDescription.toStdWString().data());
	pShortCutLink->SetShowCmd(SW_SHOW);
	// Save shelllink
	pShortCutLink->QueryInterface(&ppf);
	ppf->Save(szLnkPath.toStdWString().data(), FALSE);
    CoUninitialize();
	return TRUE;
}

void SwmgrApp::addInstaller(QJsonObject installer) {
	if (installer.isEmpty()) {
		return;
	}
	_PendingTasks.addTask(installer);
}

void SwmgrApp::downloadPoll()
{
	int startCount = 0;
	int MaxTask = 10;
	DownTaskInfo info;
	QString szTmp;
	QString defaultRepository = SwmgrApp::GetProgramProfilePath(GetSoftwareName()) + QString("Data") + QDir::separator();
	defaultRepository = QDir::toNativeSeparators(defaultRepository);

	defaultRepository = getSettingParameter(QString("Repository"), defaultRepository);

	foreach(LPDowningTaskObject taskObject, _PendingTasks.getTasks().values()) {
		if (taskObject != NULL && taskObject->hTaskHandle != NULL) {  // polling
			memset(&info, 0, sizeof(info));
			_pWapper->TaskQueryEx(taskObject->hTaskHandle, info);
			switch (info.stat)
			{
			case NOITEM:
				// maybe finish
				break;
			case TSC_ERROR:
			{
				if (info.fail_code) {
				}
			}
			break;
			case TSC_PAUSE:
				break;
			case TSC_DOWNLOAD:
				// doing
				startCount++;
				break;
			case TSC_COMPLETE:
				// finish
				break;
			case TSC_STARTPENDING:
				startCount++;
				break;
			case TSC_STOPPENDING:
				break;
			default:
				break;
			}
		}
	}

	// replenish to max task objects
	foreach(LPDowningTaskObject taskObjectReplenish, _PendingTasks.getTasks().values()) {
		if (taskObjectReplenish != NULL && taskObjectReplenish->hTaskHandle != NULL) {  // polling
			if (startCount < MaxTask) {
				startCount++;

				CommonItem TaskConfig;
				if (taskObjectReplenish->status != 0) {
					continue;
				}

				mapSoftwarePackages packages = _DataModel.getSoftPackages();
				mapSoftwarePackages::iterator iter = packages.find(taskObjectReplenish->category);
				if (iter != packages.end()) {// find category
					lstSoftwarePackage &cateList = iter.value();
					foreach(CommonItem item, cateList) {
						if (item["id"].compare(taskObjectReplenish->id,Qt::CaseInsensitive)==0) {
							TaskConfig = item;
						}
					}
				}
				if (TaskConfig.empty() || !TaskConfig.contains("downloadUrl") || TaskConfig.contains("packageName")) {
					taskObjectReplenish->status = 6;
					continue;
				}
				StrCpyW(taskObjectReplenish->downTaskparam.szTaskUrl, TaskConfig["downloadUrl"].toStdWString().data());
				StrCpyW(taskObjectReplenish->downTaskparam.szSavePath, defaultRepository.toStdWString().data());
				szTmp = defaultRepository + TaskConfig["packageName"];
				StrCpyW(taskObjectReplenish->downTaskparam.szFilename, szTmp.toStdWString().data());
				taskObjectReplenish->downTaskparam.IsOnlyOriginal = FALSE;

				HANDLE hTask = _pWapper->TaskCreate(taskObjectReplenish->downTaskparam);
				if (hTask) {
					_pWapper->TaskStart(hTask);
					taskObjectReplenish->hTaskHandle = hTask;
				}
			}
		}
	}
}

//==== for UI interface
void SwmgrApp::requestSoftCategoryList() {
    QJsonArray jsArray;
    foreach (CommonItem item,_DataModel.getSoftCategory().values()) {
        QJsonObject objParameter;
        foreach(QString key,item.keys()) {
            objParameter[key] = item.value(key);
        }

        jsArray.append(objParameter);
    }

	emit updateSoftCategory(jsArray.toVariantList());
}

void SwmgrApp::requestHotList() {
    QJsonArray jsArray;
    foreach (CommonItem item,_DataModel.getHotPackages().toList()) {
        QJsonObject objParameter;
        foreach(QString key,item.keys()) {
            objParameter[key] = item.value(key);
        }

        jsArray.append(objParameter);
    }
    emit updateHotList(jsArray.toVariantList());
}

void SwmgrApp::requestCategoryListByID(QString szCategoryID) {
    QJsonArray jsArray;
    // get someone category list
    mapSoftwarePackages::Iterator curItem = _DataModel.getSoftPackages().find(szCategoryID);
    if ( curItem != _DataModel.getSoftPackages().end() ) {
        foreach (CommonItem item,curItem.value().toList()) {
            QJsonObject objParameter;
            foreach(QString key,item.keys()) {
                objParameter[key] = item.value(key);
            }

            jsArray.append(objParameter);
        }
        emit updateCategoryListForID(szCategoryID,jsArray.toVariantList());
    }
}

void SwmgrApp::requestPackageInfoByID(QString szCategoryID,QString szPackageID) {
    QJsonObject objParameter;
    mapSoftwarePackages::Iterator curItem = _DataModel.getSoftPackages().find(szCategoryID);
    if (curItem!=_DataModel.getSoftPackages().end()) {
        lstSoftwarePackage lstSoftPkg = curItem.value();
        foreach(CommonItem it,lstSoftPkg) {
            if (it.value("id").compare(szPackageID,Qt::CaseInsensitive)==0) {
                foreach(QString key,it.keys()) {
                    objParameter[key] = it.value(key);
                }

                emit updatePackageInfoByID(objParameter.toVariantMap());
                break;
            }
        }
    }
}

void SwmgrApp::requestRegisteUser(QString username,QString password,QString email) {
    _user.RegistUser(username,password,email);
    emit updateRegisteUser(QVariant());
}

void SwmgrApp::requestCanUpdatePackages() {
	QJsonArray jsArray;

	mapSoftwareList mapSoftwares;
	OSSystemWrapper::Instance()->GetSystemInstalledSoftware(mapSoftwares,0);
	for (mapSoftwareList::iterator item = mapSoftwares.begin(); item != mapSoftwares.end();item++) {
		QJsonObject objParameter;
		for (ItemProperty::iterator it = item->second.begin(); it != item->second.end(); it++) {
			objParameter[QString::fromStdString(it->first)] = QString::fromStdString(it->second);
		}
		jsArray.append(objParameter);
	}
	emit updateCanUpdatePackages(jsArray.toVariantList());
}