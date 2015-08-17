#include "ServMain.h"
#include "SysReportLog.h"
#include "CurlEnv.h"

CServMain::CServMain() :
	m_svcStatusHandle(NULL),m_pVecEventHandle(NULL),m_cntEventHandle(0),m_cntAvalibEventHandle(0),
	m_dwCheckPoint(0),m_dwCtrlStop(0),m_hkXBSWMGR(NULL)
{
	m_svcStatus.dwServiceType = 0;
	m_svcStatus.dwCurrentState = 0;
	m_svcStatus.dwControlsAccepted = 0;
	m_svcStatus.dwWin32ExitCode = 0;
	m_svcStatus.dwServiceSpecificExitCode = 0;
	m_svcStatus.dwCheckPoint = 0;
	m_svcStatus.dwWaitHint = 0;
	m_dwCreationFlags = 0;
	ZeroMemory(&m_pi, sizeof(m_pi));
	m_hUserTokenDup = INVALID_HANDLE_VALUE;
	m_pEnv = NULL;
}
CServMain::~CServMain() {
	if (m_hUserTokenDup != INVALID_HANDLE_VALUE) CloseHandle(m_hUserTokenDup);
	if (m_pEnv != NULL) LocalFree(m_pEnv);
}
DWORD WINAPI CServMain::_SvcCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {
	CServMain *pThis = (CServMain *)lpContext;
	if (pThis == NULL)
		return 0;
	return pThis->CtrlHandler(dwControl, dwEventType, lpEventData);
}

VOID WINAPI CServMain::_ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv) {
	CServMain _servMain;
	_servMain.ServiceMain(dwArgc, lpszArgv);
}

DWORD CServMain::CtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData) {
	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
		ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
		SetEvent(m_pVecEventHandle[1]);
		return 0;
	case SERVICE_CONTROL_INTERROGATE:
		// Fall through to send current status.
		CSysReportLog::CreateInstance(_ServeName)->writeReportEvent(EVENTLOG_INFORMATION_TYPE, (LPWSTR)L"Protected service config.");
		break;
	default:
		break;
	}

	ReportSvcStatus(m_svcStatus.dwCurrentState, NO_ERROR, 0);
	return 0;
}

VOID CServMain::ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv) {
	m_dwCtrlStop = 0;
	m_dwCheckPoint = 1;
	m_svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;  //SERVICE_INTERACTIVE_PROCESS
	m_svcStatus.dwServiceSpecificExitCode = 0;
	m_svcStatusHandle = RegisterServiceCtrlHandlerExW(_ServeName.data(), CServMain::_SvcCtrlHandler, this);

	if (!m_svcStatusHandle) {
		CSysReportLog::CreateInstance(_ServeName)->writeReportEvent(EVENTLOG_ERROR_TYPE, (LPWSTR)L"at RegisterServiceCtrlHandler");
		return;
	}

	ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );
	SvcRun();
}

VOID CServMain::ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {
	m_svcStatus.dwCurrentState  = dwCurrentState;
	m_svcStatus.dwWin32ExitCode = dwWin32ExitCode;
	m_svcStatus.dwWaitHint      = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING) {
		m_svcStatus.dwControlsAccepted = 0;
	}
	else {
		m_svcStatus.dwControlsAccepted = 0;
	}

	if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED)) {
		m_svcStatus.dwCheckPoint = SERVICE_CONTROL_STOP;
	}
	else {
		m_svcStatus.dwCheckPoint = m_dwCheckPoint++;
	}

	// Report the status of the service to the SCM.
	SetServiceStatus(m_svcStatusHandle, &m_svcStatus);
}

BOOL CServMain::SysInit() {
	if (!CCurlEnv::Instance()->GetStatus()) {
		goto err;
	}
	m_pShardTaskThrdCtrl = CreateXbThread((PVOID)NULL, SoftListProc);
	if (!m_pShardTaskThrdCtrl) {
		goto err;
	}
	InitDir();
	return TRUE;
err:
	return FALSE;
}

BOOL CServMain::LoadUIEnv() {
	DWORD dwSessionId, winlogonPid;
	HANDLE hSnap = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 procEntry;  //用来存放快照进程信息的一个结构体
	HANDLE hUserToken = INVALID_HANDLE_VALUE, hProcess = INVALID_HANDLE_VALUE, hPToken = INVALID_HANDLE_VALUE/*, hUserTokenDup = INVALID_HANDLE_VALUE*/;
	TOKEN_PRIVILEGES tp;
	LUID luid;
	wchar_t Buffer[512] = { '\0' };

	// Log the client on to the local computer.
	dwSessionId = WTSGetActiveConsoleSessionId(); //得到当前用户的会话ID
	//////////////////////////////////////////
	// Find the winlogon process
	////////////////////////////////////////

	//函数为指定的进程、进程使用的堆[HEAP]、模块[MODULE]、线程[THREAD]）建立一个快照[snapshot]
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE) {
		wnsprintfW(Buffer, 512, L"CServMain::LoadUIEnv CreateToolhelp32Snapshot with %d.", GetLastError());
		goto err;
	}

	procEntry.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnap, &procEntry)) {//获得第一个进程的句柄.
		wnsprintfW(Buffer, 512, L"CServMain::LoadUIEnv Process32First with %d.", GetLastError());
		goto err;
	}
	do {
		// We found a winlogon process...make sure it's running in the console session
		if (_wcsicmp(procEntry.szExeFile, L"winlogon.exe") == 0) { //查找winlogon.exe
			DWORD winlogonSessId = 0;
			if (ProcessIdToSessionId(procEntry.th32ProcessID, &winlogonSessId) && winlogonSessId == dwSessionId) { //得到与进程ID对应的终端服务会话ID
				winlogonPid = procEntry.th32ProcessID;
				break;
			}
		}
	} while (Process32Next(hSnap, &procEntry)); //获得下一个进程的句柄

	////////////////////////////////////////////////////////////////////////
	WTSQueryUserToken(dwSessionId, &hUserToken); //通过会话ID得到令牌
	//指定新进程的主窗口特性
	//用于标识启动应用程序所在的桌面的名字。如果该桌面存在，新进程便与指定的桌面相关联。
	//如果桌面不存在，便创建一个带有默认属性的桌面，并使用为新进程指定的名字。 　　
	//如果lpDesktop是NULL（这是最常见的情况),那么该进程将与当前桌面相关联 
	if ((hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, winlogonPid))==INVALID_HANDLE_VALUE) {
		wnsprintfW(Buffer, 512, L"CServMain::LoadUIEnv OpenProcess with %d.", GetLastError());
		goto err;
	}
	if (!::OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY
		| TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID
		| TOKEN_READ | TOKEN_WRITE, &hPToken)) {                     //获得令牌句柄
		wnsprintfW(Buffer, 512, L"CServMain::LoadUIEnv OpenProcessToken with %d.", GetLastError());
		goto err;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
		wnsprintfW(Buffer, 512, L"CServMain::LoadUIEnv LookupPrivilegeValue with %d.", GetLastError());
		goto err;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	//创建模拟令牌
	if (!DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &m_hUserTokenDup)) {
		wnsprintfW(Buffer, 512, L"CServMain::LoadUIEnv DuplicateTokenEx with %d.", GetLastError());
		goto err;
	}
	//change Token session id
	SetTokenInformation(m_hUserTokenDup, TokenSessionId, (void*)dwSessionId, sizeof(DWORD));/* {
		wnsprintfW(Buffer, 512, L"CServMain::LoadUIEnv DuplicateTokenEx with %d.", GetLastError());
		goto err;
	}*/
	//Adjust Token privilege
	//这个函数启用或禁止 指定访问令牌的特权
	if (!AdjustTokenPrivileges(m_hUserTokenDup, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, NULL)) {
		wnsprintfW(Buffer, 512, L"CServMain::LoadUIEnv AdjustTokenPrivileges with %d.", GetLastError());
		goto err;
	}
	
	m_dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
	if (CreateEnvironmentBlock(&m_pEnv, m_hUserTokenDup, TRUE)) {
		m_dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
	}

	// Launch the process in the client's logon session.
	ZeroMemory(&m_si, sizeof(STARTUPINFOW));
	m_si.cb = sizeof(STARTUPINFOW);
	m_si.lpDesktop = L"winsta0\\default";

	if (hUserToken != INVALID_HANDLE_VALUE) CloseHandle(hUserToken);
	if (hPToken != INVALID_HANDLE_VALUE) CloseHandle(hPToken);
	if (hSnap != INVALID_HANDLE_VALUE) CloseHandle(hSnap);
	if (hProcess != INVALID_HANDLE_VALUE) CloseHandle(hProcess);

	return TRUE;
err:
	if (wcslen(Buffer) > 0) CSysReportLog::CreateInstance(_ServeName)->writeReportEvent(EVENTLOG_INFORMATION_TYPE, (LPWSTR)Buffer);
	if (hUserToken != INVALID_HANDLE_VALUE) CloseHandle(hUserToken);
	if (hPToken != INVALID_HANDLE_VALUE) CloseHandle(hPToken);
	if (hSnap != INVALID_HANDLE_VALUE) CloseHandle(hSnap);
	if (hProcess != INVALID_HANDLE_VALUE) CloseHandle(hProcess);
	return FALSE;
}

void CServMain::LoadUIProcess() {
	wchar_t Buffer[512] = { '\0' };
	ZeroMemory(&m_pi, sizeof(m_pi));
	if (!CreateProcessAsUserW(m_hUserTokenDup,            // client's access token
		L"D:\\workspace\\xbInst\\Debug\\xbInst.exe",     // file to execute
		NULL,			   // command line
		NULL,              // pointer to process SECURITY_ATTRIBUTES
		NULL,              // pointer to thread SECURITY_ATTRIBUTES
		FALSE,             // handles are not inheritable
		m_dwCreationFlags,   // creation flags
		m_pEnv,              // pointer to new environment block 
		NULL,              // name of current directory 
		&m_si,               // pointer to STARTUPINFO structure
		&m_pi                // receives information about new process
		)) {
		wnsprintfW(Buffer, 512, L"CServMain::LoadUIEnv CreateProcessAsUserW with %d.", GetLastError());
		goto err;
	}
err:
	if (wcslen(Buffer) > 0) CSysReportLog::CreateInstance(_ServeName)->writeReportEvent(EVENTLOG_INFORMATION_TYPE, (LPWSTR)Buffer);
}

VOID CServMain::SvcRun() {
	DWORD dwWaitRetCode = WAIT_FAILED;
	// initialize
	if (!SysInit()) {
		goto err;
	}
	if (!LoadUIEnv()) {
		goto err;
	}
	LoadUIProcess();

	m_cntEventHandle = 20;
	m_pVecEventHandle = new HANDLE[m_cntEventHandle];

	m_cntAvalibEventHandle = 0;
	m_pVecEventHandle[m_cntAvalibEventHandle++] = CreateEventW(NULL, FALSE, FALSE, NULL); // stop finish
	m_pVecEventHandle[m_cntAvalibEventHandle++] = CreateEventW(NULL, FALSE, FALSE, NULL);  // for quit cmd
	m_pVecEventHandle[m_cntAvalibEventHandle++] = m_pShardTaskThrdCtrl->m_hEvent[1]; //CreateEventW(NULL, FALSE, FALSE, NULL);

	SECURITY_DESCRIPTOR securityDesc;
	InitializeSecurityDescriptor(&securityDesc, 1);
	SECURITY_ATTRIBUTES securityAttr;
	securityAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttr.bInheritHandle = TRUE;
	securityAttr.lpSecurityDescriptor = &securityDesc;
	SetSecurityDescriptorDacl(&securityDesc, TRUE, NULL, FALSE);

	m_pVecEventHandle[m_cntAvalibEventHandle++] = CreateEventW(&securityAttr, FALSE, FALSE, wszEvtName.data());//for installer
	m_pVecEventHandle[m_cntAvalibEventHandle++] = CreateEventW(NULL, FALSE, FALSE, NULL);   //m_hkXBSWMGR;
	m_pVecEventHandle[m_cntAvalibEventHandle++] = m_pi.hProcess;

	ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

	// begin loop;
	ResumeXbThread(m_pShardTaskThrdCtrl);

	while (m_cntAvalibEventHandle > 0) {
		dwWaitRetCode = WaitForMultipleObjects(m_cntAvalibEventHandle,m_pVecEventHandle, FALSE, 1000); //1s
		if (dwWaitRetCode == WAIT_FAILED || dwWaitRetCode == WAIT_TIMEOUT) {
			continue;
		}
		else {
			dwWaitRetCode = dwWaitRetCode - WAIT_OBJECT_0;
			switch (dwWaitRetCode) {
			case 0:	// quit command
				ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0);
				goto end;
			case 1: // stop cmd send by ctrlhandler
				m_dwCtrlStop = 1;//set normal quit
				// begin to quit thread
				QuitXbThread(m_pShardTaskThrdCtrl);
				break;
			case 2: // stop cmd send by installer
				CSysReportLog::CreateInstance(_ServeName)->writeReportEvent(EVENTLOG_INFORMATION_TYPE, L"install stop cmd.");
				m_dwCtrlStop = 1;
				// all thread stopped;
				QuitXbThread(m_pShardTaskThrdCtrl);
				break;
			case 3: // thread quit
				CSysReportLog::CreateInstance(_ServeName)->writeReportEvent(EVENTLOG_INFORMATION_TYPE, L"ShareThread stopped.");
				if (m_dwCtrlStop == 1) {
					//change state to quit command
					SetEvent(m_pVecEventHandle[0]);
				}
				else {
					// thread not normal quit
					DestoryXbThread(m_pShardTaskThrdCtrl);
					m_pVecEventHandle[3] = m_pShardTaskThrdCtrl->m_hEvent[1];
				}
				break;
			case 4:
				break;
			case 5: //process exit
				if (m_pi.hProcess != INVALID_HANDLE_VALUE) CloseHandle(m_pi.hProcess);
				if (m_pi.hThread != INVALID_HANDLE_VALUE) CloseHandle(m_pi.hThread);
				if (m_dwCtrlStop != 1) {
					LoadUIProcess();
					m_pVecEventHandle[5] = m_pi.hProcess;
				}
				break;
			default:
				break;
			}
		}
	}

err:
	ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
end:
	for (DWORD i = 0; i < m_cntAvalibEventHandle; i++) {
		CloseHandle(m_pVecEventHandle[i]);
	}
   m_cntEventHandle = 0;

   if (NULL != m_pVecEventHandle) {
	   delete[]m_pVecEventHandle;
	   m_pVecEventHandle = NULL;
   }
}
