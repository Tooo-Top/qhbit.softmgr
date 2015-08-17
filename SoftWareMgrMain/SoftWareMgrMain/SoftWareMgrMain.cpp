// SoftWareMgrMain.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "servinstaller.h"
#include "servicefuncs.h"
#include "FuncsImp.h"
#include "appenv.h"
#include "CurlEnv.h"

int main(int argc, char* argv[])
{
	int nInstallMode = 0;

	if (argc == 1) {
		std::cout << "This is a service." << std::endl;
		nInstallMode = 0;
	}
	else if (argc == 2) {
		// cmd mode
		if (_strcmpi(argv[1], "install") == 0) {
			std::cout << "Will install service!" << std::endl;
			nInstallMode = 1;
		}
		else if (_strcmpi(argv[1], "normal") == 0) {
			std::cout << "Will run as normal!" << std::endl;
			nInstallMode = 2;
		}
		else {
			std::cout << "Not support command parameter!" << std::endl;
			nInstallMode = 0;
			return 1;
		}
	}
	else {
		std::cout << "Not support num of parameters!" << std::endl;
		nInstallMode = 0;
		return 1;
	}

	if (nInstallMode == 0) {
		std::cout << "will start as service!" << std::endl;
		RunServiceEntry();
		return 0;
	}
	else if (nInstallMode == 1) {
		std::cout << "will start as installer!" << std::endl;
		InstallServ();
		return 0;
	}
	else if (nInstallMode == 2) {
		// run  normal
		if (!CCurlEnv::Instance()->GetStatus()) {
			return 1;
		}
		PThreadCtrl m_pShardTaskThrdCtrl = CreateXbThread((PVOID)NULL, SoftListProc);
		InitDir();

		if (!m_pShardTaskThrdCtrl) {
			return 1;
		}
		else{
			ResumeXbThread(m_pShardTaskThrdCtrl);
			while (true) {
				//input ?
				char cmd = std::cin.get();
				//-->send command to thread
				if (cmd == 'q') {
					QuitXbThread(m_pShardTaskThrdCtrl);
					WaitForSingleObject(m_pShardTaskThrdCtrl->m_hEvent[1], INFINITE);
					break;
				}
				Sleep(1000);
			}
		}

	}
	else {
		std::cout << "unknow error!" << std::endl;
		return 1;
	}

	return 0;
}

