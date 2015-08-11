#include "widget.h"
#include <iostream>
#include <QApplication>
#include <QUrl>
#include <QSystemSemaphore>
#include "global.h"
#include "swmgrapp.h"
#include "globalsingleton.h"
#include "ConfOperation.h"

QString GLOBAL::_DY_DIR_RUNNERSELF = ' ';

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    GLOBAL::_DY_DIR_RUNNERSELF = QApplication::applicationDirPath();
    qDebug() << "_DY_DIR_RUNNERSELF:" << GLOBAL::_DY_DIR_RUNNERSELF << endl;


	QStringList lstCmd = a.arguments();
	QJsonObject jsonLaunchObject;
	bool bInstallMode = false;
	if (lstCmd.size() == 2) {
		if (lstCmd.at(1).compare("launch", Qt::CaseInsensitive) == 0) {
			bInstallMode = false;// launch application
		}
		else {
			std::cout << "arguments error!" << std::endl;
			return 1;
		}
	}
	else if (lstCmd.size() < 5) {
		std::cout << "arguments error!" << std::endl;
		return 1;
	}
	else {
		if (lstCmd.at(1).compare("install",Qt::CaseInsensitive)==0) {
			QString szArg = lstCmd.at(2);
			QStringList szIDInfo = szArg.split(QChar('='), QString::SkipEmptyParts, Qt::CaseInsensitive);
			szArg = lstCmd.at(3);
			QStringList szCatID = szArg.split(QChar('='), QString::SkipEmptyParts, Qt::CaseInsensitive);

			if (szIDInfo.size() == 2 && szIDInfo.at(0).compare("id",Qt::CaseInsensitive)==0 &&
				szCatID.size() == 2 && szCatID.at(0).compare("catid", Qt::CaseInsensitive) == 0
				) {
				jsonLaunchObject.insert(QString("id"), szIDInfo.at(1));
				jsonLaunchObject.insert(QString("catid"), szCatID.at(1));
				jsonLaunchObject.insert(QString("launchName"), lstCmd.at(4));
				qDebug() << QJsonDocument(jsonLaunchObject).toJson(QJsonDocument::Compact) << endl;
				bInstallMode = true;
			}
			else {
				std::cout << "arguments error!" << std::endl;
				return 1;
			}
		}
		else {
			std::cout << "arguments error!" << std::endl;
			return 1;
		}
	}

	GlobalSingleton *_instance = GlobalSingleton::Instance();

	// check singleton instance
	if (bInstallMode) {
		if (jsonLaunchObject.isEmpty()) {
			return 1;
		}
		else {
			// lauth first
			// write pending install profile
			QString szFile = ConfOperation::Root().getSubpathFile("Conf", "installPending.conf");
			SoftwareList::AddItemToConfArray(szFile, jsonLaunchObject);
		}

		if (_instance==NULL) { // already running
			// add install task
			// pipe notice pending install task
			SwmgrApp::NoticeMain(&a, jsonLaunchObject);
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
