#include "appenv.h"
#include "servinstaller.h"
#include "servicefuncs.h"
#include "SysReportLog.h"

bool InstallService(SC_HANDLE schSCManager, std::string servName, std::string servDisplayName, DWORD servType, DWORD servStartType, std::string servImagePath, bool replace) {
    bool bRet = false;
	std::wstring wszError;

	assert(schSCManager);
	if (CheckServiceExist(schSCManager, servName, servImagePath, servType) ) {
		if ( replace ) {
			if (!ModifyService(schSCManager, servName, servImagePath))
				goto err;
		}
	}
	else {
		if (!AddService(schSCManager, servName, servDisplayName, servType, servStartType, servImagePath)) {
			wszError.append(L"InstallService:Add service fail!");
			goto err;
		}
	}
	bRet = true;
err:
	if (wszError.size()) CSysReportLog::CreateInstance(_ServeName)->writeReportEvent(EVENTLOG_ERROR_TYPE, (LPWSTR)wszError.data());
	return bRet;
}

bool CheckServiceExist(SC_HANDLE schSCManager, std::string servName, std::string servImagePath, DWORD dwEnumServiceType) {
	bool bRet = false;
	DWORD essID = 0xFFFFFFFF;
	LPENUM_SERVICE_STATUSA SC_info_Array = NULL;
	DWORD cbBufSize = 0, dwBytesNeeded = 0, dwServicesReturned = 0;
	SC_HANDLE schService = NULL;

	std::wstring wszError;

	assert(schSCManager);
	// find service
	// for alloc buffer
	if (!EnumServicesStatusA(schSCManager, dwEnumServiceType, SERVICE_STATE_ALL, NULL, cbBufSize, &dwBytesNeeded, &dwServicesReturned, NULL)) {
		DWORD lastError = GetLastError();
		if (lastError != ERROR_MORE_DATA) {
			wchar_t Buffer[512] = { '\0' };
			wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

			wszError.append(L"CheckServiceExist:\t EnumServicesStatus() 0 error");
			wszError.append(Buffer);
			goto err;
		}
	}

	// enum service status data
	cbBufSize = dwBytesNeeded;
	dwBytesNeeded = 0;
	dwServicesReturned = 0;
	SC_info_Array = (LPENUM_SERVICE_STATUSA)LocalAlloc(LPTR, cbBufSize);
	if (!EnumServicesStatusA(schSCManager, dwEnumServiceType, SERVICE_STATE_ALL, SC_info_Array, cbBufSize, &dwBytesNeeded, &dwServicesReturned, NULL)) {
		wchar_t Buffer[512] = { '\0' };
		wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

		wszError.append(L"CheckServiceExist:\t EnumServicesStatus() 1 error");
		wszError.append(Buffer);
		goto err;
	}
	for (DWORD i = 0; i < dwServicesReturned; i++) {
		if (_stricmp(servName.data(), SC_info_Array[i].lpServiceName) == 0) {
			essID = i;
			break;
		}
	}
	if (essID != 0xFFFFFFFF) {
		bRet = true;
	}
err:
	if (wszError.size()) CSysReportLog::CreateInstance(_ServeName)->writeReportEvent(EVENTLOG_ERROR_TYPE, (LPWSTR)wszError.data());
	if (SC_info_Array) LocalFree(SC_info_Array);
	if (schService) CloseServiceHandle(schService);
	return bRet;
}

bool ModifyService(SC_HANDLE schSCManager, std::string servName, std::string servImagePath)
{
	bool bRet = false;
	DWORD errCode = 0;
	LPQUERY_SERVICE_CONFIGA SC_config = NULL;
	DWORD cbBufSize = 0, dwBytesNeeded = 0;
	SC_HANDLE schService = NULL;

	std::wstring wszError;

	assert(schSCManager);
	schService = OpenServiceA(schSCManager, servName.data(), SERVICE_ALL_ACCESS);

	if (!schService) {
		wszError.append(L"ModifyService:\0open service fail.");
		bRet = false;
		goto err;
	}
	// for alloc buffer
	if (!QueryServiceConfigA(schService, NULL, cbBufSize, &dwBytesNeeded)) {
		errCode = GetLastError();
	}
	if (errCode == ERROR_INSUFFICIENT_BUFFER) {
		cbBufSize = dwBytesNeeded;
		dwBytesNeeded = 0;
		SC_config = (LPQUERY_SERVICE_CONFIGA)LocalAlloc(LPTR, cbBufSize);
	}
	else if (errCode!=0){
		wchar_t Buffer[512] = { '\0' };
		wnsprintfW(Buffer, 512, L"ModifyService:\0QueryServiceConfigA with %d.", errCode);
		wszError.append(Buffer);
		goto err;
	}

	// query service configure parameter
	if ( !QueryServiceConfigA(schService, SC_config, cbBufSize, &dwBytesNeeded) ) {
		wchar_t Buffer[512] = { '\0' };
		wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

		wszError.append(L"ModifyService:\t QueryServiceConfig() 1 error");
		wszError.append(Buffer);
		goto err;
	}

	// modify service parameter
	if (!ChangeServiceConfigA(schService, SC_config->dwServiceType, SC_config->dwStartType, SERVICE_NO_CHANGE, 
		servImagePath.data(), SC_config->lpLoadOrderGroup, NULL, NULL, SC_config->lpServiceStartName, NULL, SC_config->lpDisplayName)) {
		wchar_t Buffer[512] = { '\0' };
		wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

		wszError.append(L"ModifyService:\t ChangeServiceConfig() error!");
		wszError.append(Buffer);
		goto err;
	}
	bRet = true;
err:
	if (wszError.size()) CSysReportLog::CreateInstance(_ServeName)->writeReportEvent(EVENTLOG_ERROR_TYPE, (LPWSTR)wszError.data());
	if (SC_config) LocalFree(SC_config);
	if (schService) CloseServiceHandle(schService);
	return bRet;
}
bool AddService(SC_HANDLE schSCManager, std::string servName, std::string servDisplayName, DWORD servType, DWORD servStartType, std::string servImagePath) {
	bool bRet = false;
	SC_HANDLE schService = NULL;

	std::wstring wszError;

	assert(schSCManager);

	// create service
	schService = CreateServiceA(schSCManager, servName.data(), servDisplayName.data(), SERVICE_ALL_ACCESS,
		servType, servStartType, SERVICE_ERROR_IGNORE, servImagePath.data(), NULL, NULL, NULL, NULL, NULL);
	if (!schService) {
		wchar_t Buffer[512] = { '\0' };
		wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

		wszError.append(L"AddService:\t CreateService() error!");
		wszError.append(Buffer);
		goto err;
	}

	bRet = true;
err:
	if (wszError.size()) CSysReportLog::CreateInstance(_ServeName)->writeReportEvent(EVENTLOG_ERROR_TYPE, (LPWSTR)wszError.data());
	if (schService) CloseServiceHandle(schService);
	return bRet;
}

bool StopService(SC_HANDLE schSCManager, std::string servName) {
	bool bRet = false;
	DWORD cbBufSize = 0, dwBytesNeeded = 0;
	DWORD errCode = 0;

	std::wstring wszError;

	LPQUERY_SERVICE_CONFIGA SC_config = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS ssp;
	DWORD dwTimeout = 30000; // 30-second time-out
	DWORD dwStartTime = GetTickCount();

	assert(schSCManager);

	schService = OpenServiceA(schSCManager, servName.data(), SERVICE_ALL_ACCESS);
	if (!schService) {
		wchar_t Buffer[512] = { '\0' };
		wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

		wszError.append(L"StopService:\t OpenService() error!");
		wszError.append(Buffer);
		goto err;
	}

	// for alloc buffer
	if (!QueryServiceConfigA(schService, NULL, cbBufSize, &dwBytesNeeded)) {
		errCode = GetLastError();
	}
	if (errCode == ERROR_INSUFFICIENT_BUFFER) {
		cbBufSize = dwBytesNeeded;
		dwBytesNeeded = 0;
		SC_config = (LPQUERY_SERVICE_CONFIGA)LocalAlloc(LPTR, cbBufSize);
	}
	else if (errCode != 0){
		wchar_t Buffer[512] = { '\0' };
		wnsprintfW(Buffer, 512, L"StopService:\tQueryServiceConfigA with %d.", errCode);
		wszError.append(Buffer);
		goto err;
	}

	if (!QueryServiceConfigA(schService, SC_config, cbBufSize, &dwBytesNeeded)) {
		wchar_t Buffer[512] = { '\0' };
		wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

		wszError.append(L"StopService:\t QueryServiceConfig() 1 error");
		wszError.append(Buffer);
		goto err;
	}

	// query service configure parameter
	if (!ControlService(schService, SERVICE_CONTROL_STOP, &ssp)) {
		wchar_t Buffer[512] = { '\0' };
		wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

		wszError.append(L"StopService:\t ControlService() error");
		wszError.append(Buffer);
		goto err;
	}
		
	while (ssp.dwCurrentState = !SERVICE_STOPPED) {
		if (!QueryServiceStatus(schService, &ssp)) {
			wchar_t Buffer[512] = { '\0' };
			wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

			wszError.append(L"StopService:\t QueryServiceStatus() error!");
			wszError.append(Buffer);
			goto err;
		}
		else if (ssp.dwCurrentState == SERVICE_STOPPED) {
			printf("StopService:\t service stopped\n");
			break;
		}
		else if ((GetTickCount() - dwStartTime) > dwTimeout) {
			printf("StopService:\t Wait timed out\n");
			break;
		}
		else {
			Sleep(1000);
		}
	}
	bRet = true;

err:
	if (wszError.size()) CSysReportLog::CreateInstance(_ServeName)->writeReportEvent(EVENTLOG_ERROR_TYPE, (LPWSTR)wszError.data());
	if (schService) CloseServiceHandle(schService);
	return bRet;
}

void StartXBService(SC_HANDLE schSCManager,std::string servName) {
    std::wstring wszError;
	SC_HANDLE schService = NULL;

	assert(schSCManager);
	schService = OpenServiceA(schSCManager, servName.data(), SERVICE_ALL_ACCESS); // Open Service Handle
	if (schService) {
		if (!StartServiceA(schService, NULL, 0)) {
			wchar_t Buffer[512] = { '\0' };
			wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

			wszError.append(L"StartXBService:\t StartService() error!");
			wszError.append(Buffer);
			goto err;
		}
		else {
			DWORD dwTimeout = 30000; // 30-second time-out
			DWORD dwStartTime = GetTickCount();

			SERVICE_STATUS ssp;
			do {
				if (!QueryServiceStatus(schService, &ssp)) {
					wchar_t Buffer[512] = { '\0' };
					wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

					wszError.append(L"StopService:\t QueryServiceStatusEx() error!");
					wszError.append(Buffer);
					goto err;
				}
				else if ((GetTickCount() - dwStartTime) > dwTimeout) {
					printf("StopService:\t Wait timed out\n");
					wszError = L"StopService:\t Wait timed out\n";
					break;
				}
				else {
					Sleep(1000);
				}
			} while (ssp.dwCurrentState != SERVICE_START_PENDING && ssp.dwCurrentState != SERVICE_RUNNING);
			// wszError = L"StartXBService:Service is run.";
		}
	}
	else {
		wchar_t Buffer[512] = { '\0' };
		wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

		wszError.append(L"StartXBService:\t StartService() error!");
		wszError.append(Buffer);
		goto err;
	}
err:
	if (wszError.size()){
		std::wcout << wszError.data() << std::endl;
		CSysReportLog::CreateInstance(_ServeName)->writeReportEvent(EVENTLOG_ERROR_TYPE, (LPWSTR)wszError.data());
	}
	if (schService) CloseServiceHandle(schService);
}

void StopXBService() {
    std::wstring szwTmp = L"Global\\";
    szwTmp.append(wszEvtName);
    HANDLE hEvent = OpenEventW(EVENT_MODIFY_STATE,FALSE,szwTmp.data());
    if (hEvent) {
		std::wcout << L"StopXBService beginning." << std::endl;
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
    else {
		std::wcout << L"StopXBService not runing." << std::endl;
    }
}

void InstallServ() {
	CHAR szPath[MAX_PATH];
	std::string szServiceName="XB_SoftwareManager";
	std::string szLoadPath;
	std::wstring wszError;
	SC_HANDLE schSCManager = NULL;

	if (!GetModuleFileNameA(NULL, szPath, MAX_PATH)) {
		wchar_t Buffer[512] = { '\0' };
		wnsprintfW(Buffer, 512, L" Cannot install service with %d.", GetLastError());

		wszError.append(Buffer);
		std::wcout << wszEvtName.data() << std::endl;
		return;
	}

	szLoadPath = GetAppdataPath("HurricaneTeam"); // Run Install
	szLoadPath.append("\\xbsoftMgr");
	_mkdir(szLoadPath.data());
	szLoadPath.append("\\SoftWareMgrMain.exe");
	StopXBService();

	schSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS); // Open SCM
	if (!schSCManager) {
		wchar_t Buffer[512] = { '\0' };
		wnsprintfW(Buffer, 512, L" with %d.", GetLastError());

		wszError.append(L"InstallService:\t OpenSCManager() == INVALID_HANDLE_VALUE error");
		wszError.append(Buffer);
		std::wcout<<wszEvtName.data()<<std::endl;
		return;
	}

	StopService(schSCManager,szServiceName);

	// repleace execute binary
	CopyFileA(szPath, szLoadPath.data(), FALSE);
	if (InstallService(schSCManager, szServiceName, szServiceName, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, SERVICE_AUTO_START, szLoadPath, true)) {
		std::wcout << L"Service is installed." << std::endl;
		StartXBService(schSCManager, szServiceName);
	}
	else{
		std::wcout << L"Service install fail." << std::endl;
	}
	
	if (schSCManager) CloseServiceHandle(schSCManager);
}

