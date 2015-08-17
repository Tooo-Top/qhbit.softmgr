#pragma once
#include "stdafx.h"
class CSysReportLog
{
protected:
	CSysReportLog(std::wstring wszModuleName);
	~CSysReportLog();
protected:
	HANDLE m_hEventSource;
	std::wstring m_wszModuleName;
	void OpenSysReportLog();
public:
	static CSysReportLog *CreateInstance(std::wstring wszModuleName);
public:
	void writeReportEvent(WORD wType, LPWSTR szMessage);
};

