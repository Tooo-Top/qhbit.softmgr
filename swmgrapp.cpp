#include "swmgrapp.h"
#include <QApplication>
#include <QMenu>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include "curl/curl.h"

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
//	wndFull = new Widget(NULL);
	srvLaunchInst = new QLocalServer(this);
	pollDownloadTaskObject = new QTimer(this);
    wndWebkit = new QWebView();
//    wndMain = new MainWnd();
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);
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
    QObject::connect(wndWebkit,SIGNAL(loadFinished(bool)),this,SLOT(docLoadFinish(bool)));
//	QObject::connect(wndMain, SIGNAL(loadFinished(bool)), this, SLOT(docLoadFinish(bool)));
//    QObject::connect(wndWebkit->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(initWebViewHost()));
//    QObject::connect(wndMain->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(initWebViewHost()));
	QObject::connect(this, SIGNAL(putSoftCategory(QVariantList)), SLOT(dumpInfo(QVariantList)));
}

void SwmgrApp::InitTray() {
	traySystem->setIcon(*appTrayIcon);
	traySystem->setContextMenu(trayIconMenu);
	traySystem->show();
}

void SwmgrApp::InitWnd() {
	//wndFull = new Widget(NULL);
    wndWebkit->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
    wndWebkit->setFixedSize(992, 613);
    wndWebkit->setMouseTracking(true);
    wndWebkit->setUrl(QUrl::fromUserInput("D:/workspace/trunk/lewang/Index.html"));
//    wndWebkit->page()->mainFrame()->addToJavaScriptWindowObject("DYBC",this);
	QObject::connect(wndWebkit->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(initWebViewHost()));

    wndWebkit->show();
//    wndMain->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
//    wndMain->setFixedSize(992, 613);
//    wndMain->setUrl(QUrl::fromUserInput("D:/workspace/trunk/lewang/Index.html"));
//    wndMain->page()->mainFrame()->addToJavaScriptWindowObject("DYBC",this);
//    wndMain->show();
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
//	_Engine->destroyed();
//	wndFull->close();
//	qApp->quit();
	QCoreApplication::exit(0);
}

void SwmgrApp::showFullWnd() {
/*    if (wndFull->isVisible())
        wndFull->hide();
    else {
        wndFull->show();
    }*/
}

void SwmgrApp::showMiniWnd() {
//    if (wndFull->isVisible())
//        wndFull->hide();
	// show mini widget
}

void SwmgrApp::initWebViewHost() {
	qDebug() << "SwmgrApp::initWebViewHost()";
    wndWebkit->page()->mainFrame()->addToJavaScriptWindowObject("DYBC",this);
}

void SwmgrApp::docLoadFinish(bool ok) {
    if (ok) {
//       QObject::connect(wndWebkit->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(initWebViewHost()));

        //wndWebkit->triggerPageAction(QWebPage::SelectAll,false);
//        wndWebkit->page()->mainFrame()->evaluateJavaScript("document.documentElement.style.webkitUserSelect='none';");
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

    emit putSoftCategory(jsArray.toVariantList());
}

void SwmgrApp::dumpInfo(QVariantList swCategory) {
//	qDebug() << swCategory;
}
