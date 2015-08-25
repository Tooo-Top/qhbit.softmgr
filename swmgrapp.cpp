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

	// ----------
    InitObjects();
    InitIcons();
    InitMenuActions();

    InitSlots();

    InitDataModel();

    InitTray();
    InitWnd();

    return TRUE;
}

void SwmgrApp::monitorProf() {
	;//
}

void SwmgrApp::NoticeMain(QObject *parent, QVariantMap &jsItem) {
    DataControl::NoticeMain(parent,jsItem);
}
