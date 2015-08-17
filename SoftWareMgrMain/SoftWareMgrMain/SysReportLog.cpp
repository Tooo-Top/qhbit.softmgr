#include "SysReportLog.h"


CSysReportLog::CSysReportLog(std::wstring wszModuleName) : m_hEventSource(NULL), m_wszModuleName(wszModuleName)
{
}


CSysReportLog::~CSysReportLog()
{
	if (m_hEventSource != NULL) {
		DeregisterEventSource(m_hEventSource);
	}
}

void CSysReportLog::OpenSysReportLog() {
	if (NULL == m_hEventSource && m_wszModuleName.size()>0) {
		m_hEventSource = RegisterEventSourceW(NULL, m_wszModuleName.data());
	}
	if (m_hEventSource == NULL)
		return;
}

CSysReportLog *CSysReportLog::CreateInstance(std::wstring wszModuleName) {
	static CSysReportLog obj(wszModuleName);
	obj.OpenSysReportLog();
	return &obj;
}

void CSysReportLog::writeReportEvent(WORD wType, LPWSTR szMessage) {
	LPCWSTR lpszStrings[2];
	if (m_hEventSource == NULL)
		return;

	lpszStrings[0] = m_wszModuleName.data();
	lpszStrings[1] = szMessage;

	ReportEvent(m_hEventSource, wType, 0, 0, NULL, 2, 0, lpszStrings, NULL);
}