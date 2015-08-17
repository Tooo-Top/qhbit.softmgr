#pragma once
#include "stdafx.h"
#include "appenv.h"
#include "FuncsImp.h"

// -----------------------------------------

class CServMain
{
protected:
	CServMain();
	~CServMain();
protected:
	SERVICE_STATUS_HANDLE   m_svcStatusHandle;
	SERVICE_STATUS          m_svcStatus;
	HANDLE                 *m_pVecEventHandle;
	DWORD                   m_cntEventHandle;
	DWORD                   m_cntAvalibEventHandle;
	DWORD                   m_dwCheckPoint;
	DWORD                   m_dwCtrlStop;
	HKEY                    m_hkXBSWMGR;
	PThreadCtrl             m_pShardTaskThrdCtrl;
	PROCESS_INFORMATION     m_pi;
	STARTUPINFOW            m_si;
	DWORD                   m_dwCreationFlags;
	HANDLE                  m_hUserTokenDup;
	LPVOID                  m_pEnv;

public:
	static DWORD WINAPI _SvcCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
	static VOID WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
protected:
	DWORD CtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData);
	VOID ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
	VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
	VOID SvcRun();
protected:
	BOOL SysInit();
	BOOL LoadUIEnv();
	void LoadUIProcess();
};

