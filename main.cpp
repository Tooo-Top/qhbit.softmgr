#include <iostream>
#include <QtDebug>
#include <QApplication>
#include <QResource>
#include <QUrl>
#include <QSystemSemaphore>
#include <QJsonDocument>
#include "global.h"
#include "Storage.h"
#include "globalsingleton.h"
#include "ConfOperation.h"
#include "swmgrapp.h"
#include "CommandLine.h"

QString GLOBAL::_DY_DIR_RUNNERSELF = ' ';

void adjustProcessPrivilege() {
	HANDLE hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, GetCurrentProcessId());
	HANDLE hPToken = INVALID_HANDLE_VALUE;
	LUID luid;
	TOKEN_PRIVILEGES tp;
	if (hProcess != INVALID_HANDLE_VALUE &&
		::OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID | TOKEN_READ | TOKEN_WRITE, &hPToken)) {
		//获得令牌句柄
		if (hPToken != INVALID_HANDLE_VALUE && LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = luid;
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			if (AdjustTokenPrivileges(hPToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, NULL)) {
				qDebug() << "adjust process privileges.";
			}
			else {
				qDebug() << GetLastError();
			}
		}
		else {
			;
		}
	}
	else {
		;
	}
}

int main(int argc, char *argv[])
{
	adjustProcessPrivilege();
    QApplication a(argc, argv);

    GLOBAL::_DY_DIR_RUNNERSELF = QApplication::applicationDirPath();
    if(QFile::exists(GLOBAL::_DY_DIR_RUNNERSELF+"/essential.res")){
        QResource::registerResource(GLOBAL::_DY_DIR_RUNNERSELF+"/essential.res");
    }else{
        qDebug() << "LOAD CORE RESOURCE FAIL:" << GLOBAL::_DY_DIR_RUNNERSELF+"/essential.res" << endl;
        QApplication::quit();
    }

    /*
     * Add lib path, must be it!
     * Because production env not find plugins for qt plugin window dll.
     * PS: Or use qt.conf fix it problem. Recommend use QApplication::addLibraryPath
     */
    QApplication::addLibraryPath(GLOBAL::_DY_DIR_RUNNERSELF);
    QApplication::addLibraryPath(GLOBAL::_DY_DIR_RUNNERSELF + "/plugins");
    //qDebug() << "_DY_DIR_RUNNERSELF:" << GLOBAL::_DY_DIR_RUNNERSELF << endl;

	CommandLine cmdLine;
	if (cmdLine.parseCommandLine(a.arguments()) != 0)
		return -1;
	QVariantMap launchObject = cmdLine.encodeToVariantMap();

	GlobalSingleton *_instance = GlobalSingleton::Instance();

	// check singleton instance
	if (cmdLine.launchMode()) {
		// write pending install profile
		QString szFile = ConfOperation::Root().getSubpathFile("Conf", "installPending.conf");
		Storage::AddItemToConfArray(szFile, launchObject);

		if (_instance==NULL) { // already running
			// add install task
			// pipe notice pending install task
			SwmgrApp::NoticeMain(&a, launchObject);
			return 0;
		}
	}
	else if (_instance==NULL) {
		std::cout << "instance already exist!" << std::endl;
		return 1;
	}

	if (!SwmgrApp::Instance()->InitAppEnv()){
        std::cout << "Initial error!" << std::endl;
		return 1;
	}

    return a.exec();
}
