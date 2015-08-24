#include "swmgrapp.h"
#include <QApplication>
#include "curl/curl.h"
#include "global.h"

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

BOOL SwmgrApp::InitAppEnv() {
    InitDir(SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()));
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

    InitDataModel();

    InitTray();
    InitWnd();

    StartPoll();

    return TRUE;
}

void SwmgrApp::StartPoll() {
	pollDownloadTaskObject->start(500);
}

void SwmgrApp::UninitEnv() {

}

void SwmgrApp::monitorProf() {
	;//
}

void SwmgrApp::NoticeMain(QObject *parent, QVariantMap &jsItem) {
    DataControl::NoticeMain(parent,jsItem);
}

void SwmgrApp::downloadPoll()
{
	int startCount = 0;
	int MaxTask = 10;
	DownTaskInfo info;
	QString szTmp;
	QString defaultRepository = SwmgrApp::GetProgramProfilePath(GetSoftwareName()) + QString("Data") + QDir::separator();
	defaultRepository = QDir::toNativeSeparators(defaultRepository);

	defaultRepository = _DataModel->getSettingParameter(QString("Repository"), defaultRepository);

//	foreach(LPDowningTaskObject taskObject, _PendingTasks.getTasks().values()) {
//		if (taskObject != NULL && taskObject->hTaskHandle != NULL) {  // polling
//			memset(&info, 0, sizeof(info));
//			_pWapper->TaskQueryEx(taskObject->hTaskHandle, info);
//			switch (info.stat)
//			{
//			case NOITEM:
//				// maybe finish
//				break;
//			case TSC_ERROR:
//			{
//				if (info.fail_code) {
//				}
//			}
//			break;
//			case TSC_PAUSE:
//				break;
//			case TSC_DOWNLOAD:
//				// doing
//				startCount++;
//				break;
//			case TSC_COMPLETE:
//				// finish
//				break;
//			case TSC_STARTPENDING:
//				startCount++;
//				break;
//			case TSC_STOPPENDING:
//				break;
//			default:
//				break;
//			}
//		}
//	}
//
//	// replenish to max task objects
//	foreach(LPDowningTaskObject taskObjectReplenish, _PendingTasks.getTasks().values()) {
//		if (taskObjectReplenish != NULL && taskObjectReplenish->hTaskHandle != NULL) {  // polling
//			if (startCount < MaxTask) {
//				startCount++;
//
///*				CommonItem TaskConfig;
//				if (taskObjectReplenish->status != 0) {
//					continue;
//				}
//
//				mapSoftwarePackages packages = _DataModel.getSoftPackages();
//				mapSoftwarePackages::iterator iter = packages.find(taskObjectReplenish->category);
//				if (iter != packages.end()) {// find category
//					lstSoftwarePackage &cateList = iter.value();
//					foreach(CommonItem item, cateList) {
//						if (item["id"].compare(taskObjectReplenish->id,Qt::CaseInsensitive)==0) {
//							TaskConfig = item;
//						}
//					}
//				}
//				if (TaskConfig.empty() || !TaskConfig.contains("downloadUrl") || TaskConfig.contains("packageName")) {
//					taskObjectReplenish->status = 6;
//					continue;
//				}
//				StrCpyW(taskObjectReplenish->downTaskparam.szTaskUrl, TaskConfig["downloadUrl"].toStdWString().data());
//				StrCpyW(taskObjectReplenish->downTaskparam.szSavePath, defaultRepository.toStdWString().data());
//				szTmp = defaultRepository + TaskConfig["packageName"];
//				StrCpyW(taskObjectReplenish->downTaskparam.szFilename, szTmp.toStdWString().data());
//				taskObjectReplenish->downTaskparam.IsOnlyOriginal = FALSE;
//
//				HANDLE hTask = _pWapper->TaskCreate(taskObjectReplenish->downTaskparam);
//				if (hTask) {
//					_pWapper->TaskStart(hTask);
//					taskObjectReplenish->hTaskHandle = hTask;
//				}*/
//			}
//		}
//	}
}

