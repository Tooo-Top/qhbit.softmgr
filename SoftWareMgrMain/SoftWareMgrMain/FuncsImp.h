#pragma once
#include "stdafx.h"
#include "curl/curl.h"
#include "json/json.h"
#include "json/json/writer.h"
const int categoryitem_count = 4;
const int packageitem_count = 28;
const int userinfoitem_count = 13;
typedef struct _tagThreadCtrl {
	HANDLE m_hThread;
	HANDLE m_hEvent[2];//0:quit;1:resolve;
	HANDLE m_hPipe[2]; //communication to thread;

	PVOID  m_Parameter;
	DWORD(*ThreadProc)(_tagThreadCtrl*);
	DWORD m_dwThreadId;
	WORD  m_wStatus;
}ThreadCtrl, *PThreadCtrl;

typedef struct _tagUserInfo {
	std::string init;
	std::string userName;
	std::string userPassword;
	std::string userToken;
	//person private info
	std::map<std::string, std::string> userPrivateInfo;
} UserInfo, *LPUserInfo;

typedef std::map<std::string, std::string> commonItems;
typedef std::vector<commonItems>           lstPackageDetails;
//
extern std::string categoryItems[categoryitem_count];
extern std::string packageItems[packageitem_count];
extern std::string userinfoItem[userinfoitem_count];
extern Json::Value _jsSoftCategory;
extern Json::Value _jsTopCategory;
extern Json::Value _jsHotCategory;
// default category "top"

// thread functions
DWORD WINAPI CommonThreadProc(LPVOID);
PThreadCtrl CreateXbThread(PVOID, DWORD(*ThreadProc)(PThreadCtrl));
BOOL ResumeXbThread(PThreadCtrl);
VOID QuitXbThread(PThreadCtrl);
VOID DestoryXbThread(PThreadCtrl);
//==========================================

// function implements
//==========================================

BOOL GetResourceFromHttp(const char *urls, const char *filename);
BOOL GetConfFromServ(std::string &serverUrl, std::string &fileName);

void software_cache_init(int refresh=0);
void software_cache_load();
void software_cache_idle(bool &bOver);
void software_cache_idle_lower();

BOOL chkCategoryList(std::string szFileName);
BOOL chkCategoryListDetails(std::string szFileName);
BOOL chkCategoryPackageDetails(std::string szFileName);
BOOL chkCategoryPackageDetailItem(Json::Value &__jsonPkgItem);

int GetUserInfo(LPUserInfo *lpUserInfo);
DWORD SoftListProc(PThreadCtrl);

int FetchSoftList(LONGLONG version, std::string szUrls, int nType, LONGLONG nSubID, std::string optParam);
void FetchConfigList();

//BOOL ParseCategoryList(mapCategory &category);
//BOOL ParsePackageList(std::string nCategory, lstPackageDetails &lstPackage);
BOOL FetchPackageData(int type, std::string szUrl, std::string fileName);
BOOL AddToIconsRepository(std::string nCategoryID, Json::Value &item, std::string szIconFile);
BOOL CreateShellLinkForRecommend(lstPackageDetails lstPackages);
BOOL UpdateShellLinkOfRepository(commonItems &item);

BOOL LoadTopConfigList(lstPackageDetails &lstPackages);
BOOL UserLoginRestore();
bool CreateShellLink(std::string szTargetExec, std::string szID, std::string szCatID, std::string szLnkName, std::string szIconName, std::string szDescription);
