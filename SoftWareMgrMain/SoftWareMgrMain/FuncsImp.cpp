#include "FuncsImp.h"
#include "appenv.h"

//mapCategory _map_category;
//mapIcons _map_app_icons;

std::string categoryItems[categoryitem_count] = {
	"id",
	"name",
	"alias",
	"total"
};

std::string packageItems[packageitem_count] = {
	"id",
	"packageName",
	"windowsVersion",
	"arch",
	"name",
	"category",
	"description",
	"developer",
	"iconUrl",
	"largeIcon",

	"screenshotsUrl",
	"incomeShare",
	"rating",
	"versionName",
	"versionCode",
	"priceInfo",
	"tag",
	"downloadUrl",
	"hash",
	"size",

	"createTime",
	"updateTime",
	"signature",
	"updateInfo",
	"language",
	"brief",
	"isAd",
	"status"
};

std::string userinfoItem[userinfoitem_count] = {
	"address",
	"cents",
	"ctime",
	"email",
	"email_valid",

	"gender",
	"id",
	"ltime",
	"mobile",
	"mobile_valid",

	"mtime",
	"nickname",
	"status"
};

DWORD WINAPI CommonThreadProc(LPVOID Parameter) {
	PThreadCtrl pThreadCtrl = (PThreadCtrl)Parameter;
	if (!Parameter)
		return 1;
	return pThreadCtrl->ThreadProc(pThreadCtrl);
}

PThreadCtrl CreateXbThread(PVOID Parameter, DWORD(*ThreadProc)(PThreadCtrl)){
	int i = 0;
	DWORD dwThreadId = 0;
	HANDLE hThread = INVALID_HANDLE_VALUE;
	HANDLE hPipe[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
	HANDLE hEvent[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };

	PThreadCtrl pThreadCtrl = new ThreadCtrl;
	if (!ThreadProc)
		return NULL;
	if (!CreatePipe(&hPipe[0], &hPipe[1], NULL, 1024)) {
		return NULL;
	}
	for (i = 0; i < 2; i++) {
		hEvent[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	hThread = CreateThread(NULL, 0, CommonThreadProc, (PVOID)pThreadCtrl, CREATE_SUSPENDED, &dwThreadId);
	if (hPipe[0] == INVALID_HANDLE_VALUE ||
		hPipe[1] == INVALID_HANDLE_VALUE ||
		hEvent[0] == INVALID_HANDLE_VALUE ||
		hEvent[1] == INVALID_HANDLE_VALUE ||
		hThread == INVALID_HANDLE_VALUE
		) {
		// Initial fail
		for (i = 0; i < 2; i++) {
			if (hPipe[i] != INVALID_HANDLE_VALUE)
				CloseHandle(hPipe[i]);
			if (hEvent[i] != INVALID_HANDLE_VALUE)
				CloseHandle(hEvent[i]);
		}
		if (hThread != INVALID_HANDLE_VALUE) {
			TerminateThread(hThread, 0);
			CloseHandle(hThread);
		}
		return NULL;
	}
	else {
		pThreadCtrl->m_hThread = hThread;
		for (i = 0; i < 2; i++) {
			pThreadCtrl->m_hEvent[i] = hEvent[i];
			pThreadCtrl->m_hPipe[i] = hPipe[i];
		}
		pThreadCtrl->m_Parameter = Parameter;
		pThreadCtrl->ThreadProc = ThreadProc;
		pThreadCtrl->m_dwThreadId = dwThreadId;
		pThreadCtrl->m_wStatus = 0;
		return pThreadCtrl;
	}
}

BOOL ResumeXbThread(PThreadCtrl pThreadCtrl) {
	if (!pThreadCtrl) {
		return FALSE;
	}
	if (pThreadCtrl->m_wStatus == 0) {
		pThreadCtrl->m_wStatus = 1;
		ResumeThread(pThreadCtrl->m_hThread);
		return TRUE;
	}
	return FALSE;
}

VOID QuitXbThread(PThreadCtrl pThrdCtrl) {
	if (!pThrdCtrl)
		return;
	SetEvent(pThrdCtrl->m_hEvent[0]);
}

VOID DestoryXbThread(PThreadCtrl pThrdCtrl) {
	if (!pThrdCtrl)
		return;

	if (pThrdCtrl->m_hThread != INVALID_HANDLE_VALUE) {
		CloseHandle(pThrdCtrl->m_hThread);
		pThrdCtrl->m_hThread = INVALID_HANDLE_VALUE;
	}
	for (int i = 0; i < 2; i++) {
		if (pThrdCtrl->m_hEvent[i] = INVALID_HANDLE_VALUE) {
			CloseHandle(pThrdCtrl->m_hEvent[i]);
			pThrdCtrl->m_hEvent[i] = INVALID_HANDLE_VALUE;
		}
		if (pThrdCtrl->m_hPipe[i] = INVALID_HANDLE_VALUE) {
			CloseHandle(pThrdCtrl->m_hPipe[i]);
			pThrdCtrl->m_hPipe[i] = INVALID_HANDLE_VALUE;
		}
	}
	delete pThrdCtrl;
}

struct jsItemIncheck : public std::binary_function <std::string, Json::Value, bool > {
public:
	result_type operator() (first_argument_type name, second_argument_type object) const {
		if (object.isMember(name) && object[name].isString()) {
			return true;
		}
		return false;
	}
};

size_t GetMethodCallback(char *buffer, size_t size, size_t nitems, void *outstream) {
	std::string *pszBuf = (std::string *)outstream;
	pszBuf->append(buffer, size * nitems);
	return size * nitems;
}

int GetUserInfo(LPUserInfo pUserInfo) {
	std::string BaseDir;
	std::ifstream fJson;
	Json::Reader r;
	Json::Value	__jsonRoot(Json::objectValue);

	// clear status;
	pUserInfo->init = std::string("0xff");
	pUserInfo->userName = std::string("");
	pUserInfo->userPassword = std::string("");
	pUserInfo->userToken = std::string("");
	pUserInfo->userPrivateInfo.clear();// last 13 element

	BaseDir = GetProgramProfilePath("xbsoftMgr");
	BaseDir.append("\\Data\\user.dat");
	if (!PathFileExistsA(BaseDir.data())) {
		return 0xff;
	}

	fJson.open(BaseDir);
	if (!fJson.is_open() || !r.parse(fJson, __jsonRoot, false) || !__jsonRoot.isObject()) {
		return 0xff;
	}
	std::string items[4] = { "init","username", "password", "token" };

	if (!std::any_of(&items[0], &items[1], std::bind2nd(jsItemIncheck(), __jsonRoot))) {
		pUserInfo->init = std::string("0xff");
		return 0xff;
	}
	else {
		pUserInfo->init = __jsonRoot["init"].asString();
	}
	if (!std::any_of(&items[1], &items[2], std::bind2nd(jsItemIncheck(), __jsonRoot))) {
		pUserInfo->init = std::string("0");
		return 0;
	}
	else {
		pUserInfo->userName = __jsonRoot["username"].asString();
	}

	if (!std::any_of(&items[2], &items[3], std::bind2nd(jsItemIncheck(), __jsonRoot))) {
		pUserInfo->init = std::string("1");
		return 0;
	}
	else {
		pUserInfo->userPassword = __jsonRoot["password"].asString();
	}

	if (!std::any_of(&items[3], &items[4], std::bind2nd(jsItemIncheck(), __jsonRoot))) {
		pUserInfo->init = std::string("2");
		return 0;
	}
	else {
		pUserInfo->userToken = __jsonRoot["token"].asString();
	}

	for (int i = 0; i < userinfoitem_count; i++){
		pUserInfo->userPrivateInfo[userinfoItem[i]] = __jsonRoot[userinfoItem[i]].asString();
	}
	return 0;
}

int SaveUserInfo(LPUserInfo pUserInfo) {
	std::string BaseDir;
	std::ofstream ofJson;
	Json::FastWriter writer;
	Json::Value	__jsonRoot(Json::objectValue);

	if (pUserInfo == NULL) {
		return -1;
	}
	BaseDir = GetProgramProfilePath("xbsoftMgr");
	BaseDir.append("\\Data\\user.dat");

	__jsonRoot["username"]=pUserInfo->userName;
	__jsonRoot["password"] = pUserInfo->userPassword;
	__jsonRoot["init"] = pUserInfo->init;
	__jsonRoot["token"] = pUserInfo->userToken;
	for (int i = 0; i < userinfoitem_count; i++) {
		__jsonRoot[userinfoItem[i]] = pUserInfo->userPrivateInfo[userinfoItem[i]];
	}
	ofJson.open(BaseDir.data(), std::ios_base::trunc);
	if (ofJson.is_open()) {
		Json::FastWriter w;
		ofJson << w.write(__jsonRoot);
		ofJson.close();
		return 0;
	}
	return -1;
}

int tickUserTokon(LPUserInfo pUserInfo) {
	std::string szHttpGet = "http://ctr.datacld.com/api/user?token=";
	std::string szCookieFile = GetProgramProfilePath("xbsoftMgr") + "\\xbsoftMgr.cookie";
	std::string szBuf;
	int nRetval = -1;
	if (pUserInfo->userToken.size() > 0) {
		szHttpGet.append(pUserInfo->userToken);
		std::cout << "tickUserTokon :url=" << szHttpGet << std::endl;
		CURL* curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, szHttpGet.data());
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, szCookieFile.data());
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, szCookieFile.data());
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)5); // 5s
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GetMethodCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &szBuf);

		CURLcode res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (res == CURLE_OK) {
			Json::Reader r;
			Json::Value retObject;
			if (r.parse(szBuf, retObject)) {
				if (retObject.isMember("code") && retObject["code"].isInt() && retObject.isMember("msg") && retObject["msg"].isInt()) {
					if (retObject["code"].asInt() == 0) {
						nRetval = retObject["msg"].asInt()*1000;
					}
					else if (retObject["code"].asInt() == 1) {
						pUserInfo->userToken = "";
						std::cout << "tickUserTokon : msg field=" << retObject["msg"].asInt() << std::endl;
					}
				}
				else {
					pUserInfo->userToken = "";
					std::cout << "tickUserTokon : message format error!" << std::endl;
				}
			}
		}
	}
	if (nRetval != -1) {
		SaveUserInfo(pUserInfo);//save into user.dat
	}
	return nRetval;
}

int autoUserLogin(LPUserInfo pUserInfo) {
	std::string url = "http://ctr.datacld.com/api/user";
	std::string szCookieFile = GetProgramProfilePath("xbsoftMgr") + "\\xbsoftMgr.cookie";
	std::string response;
	int nRetval = -1;
	char post[1024] = { '\0' };
	if (pUserInfo == NULL) {
		return nRetval;
	}
	sprintf_s(post, "username=%s&password=%s", pUserInfo->userName.c_str(), pUserInfo->userPassword.c_str());
	CURL* curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.data());
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, szCookieFile.data());
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, szCookieFile.data());
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)5);//5s
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(post));
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GetMethodCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		curl_easy_setopt(curl, CURLOPT_POST, (long)1);

		CURLcode res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (res == CURLE_OK) {
			Json::Reader r;
			Json::Value retObject;
			if (r.parse(response, retObject)) {
				if (retObject.isMember("code") && retObject["code"].isInt() && retObject.isMember("msg") && retObject["msg"].isObject()) {
					if (retObject["code"].asInt() == 0) {
						nRetval = 1200000; // 20min
						pUserInfo->init = "2";
						if (retObject["msg"].isMember("token")) {
							pUserInfo->userToken = retObject["msg"].isMember("token")&&retObject["msg"]["token"].isString() ? retObject["msg"]["token"].asString() : "";
							//std::cout << "autoUserLogin : msg field=" << pUserInfo->userToken << std::endl;
						}
						for (int i = 0; i < userinfoitem_count; i++){
							pUserInfo->userPrivateInfo[userinfoItem[i]] = retObject["msg"].isMember(userinfoItem[i]) && retObject["msg"][userinfoItem[i]].isString() ? retObject["msg"][userinfoItem[i]].asString() : "";
						}
					}
					else {
						pUserInfo->init = "1";
						pUserInfo->userPassword = "";
					}
				}
			}
		}
	}
	if (nRetval != -1) {
		SaveUserInfo(pUserInfo);//save into user.dat
	}
	return nRetval;
}

void StartDirMonitor(std::string &userDatPath,char *chBuf,DWORD chSize, DWORD &dwDirNotify,HANDLE evtSignal, HANDLE &hDir, OVERLAPPED &op) {
	op.hEvent = evtSignal;
	op.Internal = 0;
	op.InternalHigh = 0;
	op.Offset = 0;
	op.OffsetHigh = 0;
	dwDirNotify = 0;
	memset(chBuf, '\0', chSize);
	ReadDirectoryChangesW(hDir, chBuf, chSize, FALSE, FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE, &dwDirNotify, &op, NULL);
}

DWORD SoftListProc(PThreadCtrl pThrdCtrl) {
	bool bOpenFileMonitor = false; // for need open monitor user.dat
	bool bCreateShellLink = false;// for check shell link -- create
	bool bConfigInited = false;

	bool autoLogin = false;  // auto login
	bool userContinueToken = false;    // continue token

	DWORD waitRet = WAIT_FAILED;

	DWORD dwConstPerTimeWait = 10;         // 100 ms;
	DWORD dwConstIdle = 100;

	LONGLONG dwConstWaitTimerUse = -10000; //1 ms

	LONG dwConstTickTokenWait = 20*60*1000;   // 20 min  for continue token
	LONG dwConstAutoLoginTimeWait = 3*60*1000;// 3 min
	LONG dwConstUpdateSoftList = 60*60*1000;  // 1 hour

	DWORD dwTickCountCur = GetTickCount();   // current time

	DWORD dwTickLoopIdle = dwTickCountCur; // for common idle
	DWORD dwTickLoopIdleLower = dwTickCountCur; // for lower common idle

	UserInfo userInfo,priv_userInfo;
	lstPackageDetails lstUserPackages;

	OVERLAPPED op;
	HANDLE hDir = INVALID_HANDLE_VALUE;
	DWORD dwDirNotify = 0;
	char chBuf[1024] = { '\0' };

	int nNextTickToken = -1;
	std::vector<std::string> iconDownTask;
	std::string userDatPath = GetProgramProfilePath("xbsoftMgr") + "\\Data";

	_mkdir(userDatPath.data()); // path is exist. sure
	unsigned char waitHandleCnt = 0;
	HANDLE waitHandles[20] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
	waitHandles[0] = pThrdCtrl->m_hEvent[0]; waitHandleCnt++;
	waitHandles[1] = CreateEvent(NULL, TRUE, FALSE, NULL); waitHandleCnt++;
	waitHandles[2] = CreateEvent(NULL, TRUE, FALSE, NULL); waitHandleCnt++;
	waitHandles[3] = CreateEvent(NULL, TRUE, FALSE, NULL); waitHandleCnt++;
	waitHandles[4] = CreateEvent(NULL, TRUE, FALSE, NULL); waitHandleCnt++;
	waitHandles[5] = CreateEvent(NULL, TRUE, TRUE, NULL);  waitHandleCnt++;//load user info
	waitHandles[6] = CreateEvent(NULL, TRUE, FALSE, NULL); waitHandleCnt++;
	waitHandles[7] = CreateEvent(NULL, TRUE, FALSE, NULL); waitHandleCnt++;
	waitHandles[8] = CreateEvent(NULL, TRUE, FALSE, NULL); waitHandleCnt++;
	waitHandles[9] = CreateEvent(NULL, TRUE, FALSE, NULL); waitHandleCnt++;
	waitHandles[10] = CreateWaitableTimer(NULL, FALSE, NULL); waitHandleCnt++;
	waitHandles[11] = CreateWaitableTimer(NULL, FALSE, NULL); waitHandleCnt++;
	waitHandles[12] = CreateWaitableTimer(NULL, FALSE, NULL); waitHandleCnt++;
	LARGE_INTEGER liDueTimeer_1, liDueTimeer_2, liDueTimeer_3;

	liDueTimeer_1.QuadPart = dwConstWaitTimerUse *dwConstAutoLoginTimeWait;
	SetWaitableTimer(waitHandles[10], &liDueTimeer_1, dwConstAutoLoginTimeWait, NULL, NULL, FALSE);
	
	liDueTimeer_2.QuadPart = dwConstWaitTimerUse *dwConstTickTokenWait;
	SetWaitableTimer(waitHandles[11], &liDueTimeer_2, dwConstTickTokenWait, NULL, NULL, FALSE);
	
	liDueTimeer_3.QuadPart = dwConstWaitTimerUse *dwConstUpdateSoftList;
	SetWaitableTimer(waitHandles[12], &liDueTimeer_3, dwConstUpdateSoftList, NULL, NULL, FALSE);
	
	software_cache_init();
	while (true) { // run loop
		if (bOpenFileMonitor) {
			bOpenFileMonitor = false;
			if (hDir == INVALID_HANDLE_VALUE) {
				_mkdir(userDatPath.data());
				hDir = ::CreateFileA(userDatPath.data(), GENERIC_READ | GENERIC_WRITE | FILE_LIST_DIRECTORY,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 
					FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
				StartDirMonitor(userDatPath, (char *)chBuf, 1024, dwDirNotify, waitHandles[1], hDir, op);
			}
		}

		waitRet = WaitForMultipleObjects(waitHandleCnt, waitHandles, FALSE, dwConstPerTimeWait);  // per 1min
		dwTickCountCur = GetTickCount();
		if (waitRet == WAIT_FAILED){ break; }
		else if (waitRet == WAIT_OBJECT_0) { ResetEvent(waitHandles[0]); break; }
		else if (waitRet == WAIT_OBJECT_0 + 1) { // monitor user.data be modify
			ResetEvent(waitHandles[1]);

			PFILE_NOTIFY_INFORMATION pFNI = (PFILE_NOTIFY_INFORMATION)chBuf;
			if (pFNI->Action == FILE_ACTION_MODIFIED && pFNI->NextEntryOffset == 0) {
				if (_wcsicmp(pFNI->FileName, L"user.dat") == 0) {
					std::wcout << "pFNI->NextEntryOffset:" << pFNI->NextEntryOffset << L";FILE_ACTION_MODIFIED:" << pFNI->FileName << std::endl;
					SetEvent(waitHandles[5]);
				}
				else if (_wcsicmp(pFNI->FileName, L"userRestoreSoft.list") == 0) {
					std::wcout << "pFNI->NextEntryOffset:" << pFNI->NextEntryOffset << L";FILE_ACTION_MODIFIED:" << pFNI->FileName << std::endl;
					SetEvent(waitHandles[6]);
				}
			}
			StartDirMonitor(userDatPath, (char *)chBuf, 1024, dwDirNotify, waitHandles[1], hDir, op);
		}
		else if (waitRet == WAIT_OBJECT_0 + 2) { //update config
			ResetEvent(waitHandles[2]);
			std::cout << "update config" << std::endl;

			FetchConfigList();
			software_cache_init(1);
			bConfigInited = false;
			liDueTimeer_3.QuadPart = dwConstWaitTimerUse * dwConstUpdateSoftList; 
			SetWaitableTimer(waitHandles[12], &liDueTimeer_3, dwConstUpdateSoftList, NULL, NULL, FALSE);
			bCreateShellLink = true;
		}
		else if (waitRet == WAIT_OBJECT_0 + 3) {
			ResetEvent(waitHandles[3]);
			bool status = false;
			software_cache_idle(status);
			if (!bConfigInited) {
				if (status) {
					bConfigInited = true;
					SetEvent(waitHandles[6]);
				}
			}
		}
		else if (waitRet == WAIT_OBJECT_0 + 4) {
			ResetEvent(waitHandles[4]);
			software_cache_idle_lower();
		}
		else if (waitRet == WAIT_OBJECT_0 + 5) { 
			// use to load user info 
			ResetEvent(waitHandles[5]);
			std::cout << "use to load user info" << std::endl;
			if (hDir != INVALID_HANDLE_VALUE) {
				CloseHandle(hDir);
				ResetEvent(waitHandles[1]);
				hDir = INVALID_HANDLE_VALUE;
				bOpenFileMonitor = false;
			}
			priv_userInfo = userInfo;

			if (GetUserInfo(&userInfo) == 0){
				if (priv_userInfo.init.compare(userInfo.init) != 0) { //status changed.
					if (userInfo.init.compare("1") == 0) { // username and password is initialize need auto login
						// change user name or password ??
						if (priv_userInfo.userName.compare(userInfo.userName) != 0 || priv_userInfo.userPassword.compare(userInfo.userPassword)!=0) {
							//==start auto login==
							SetEvent(waitHandles[7]);
						}
					}
					else if (userInfo.init.compare("2") == 0) { // user already login need continue token
						// change user name or password ??
						if (priv_userInfo.userName.compare(userInfo.userName) != 0 || priv_userInfo.userPassword.compare(userInfo.userPassword)!=0) {
							//==start auto login==
							SetEvent(waitHandles[7]);
						}
						else {
							userContinueToken = true;
							//SetEvent(waitHandles[8]);
							liDueTimeer_2.QuadPart = dwConstWaitTimerUse * dwConstTickTokenWait; //1s
							SetWaitableTimer(waitHandles[11], &liDueTimeer_2, dwConstTickTokenWait, NULL, NULL, FALSE);
						}
					}
				}
			}
			bOpenFileMonitor = true;
		}
		else if (waitRet == WAIT_OBJECT_0 + 6) {
			// load reload desktop shelllink define
			ResetEvent(waitHandles[6]);
			std::cout << "load reload desktop shelllink define" << std::endl;
			lstUserPackages.clear();
			if (UserLoginRestore()) {
				;// load userself software list
			}
			else {
				LoadTopConfigList(lstUserPackages);
			}
			bCreateShellLink = true;
		}
		else if (waitRet == WAIT_OBJECT_0 + 7){
			//use to auto login
			ResetEvent(waitHandles[7]);
			std::cout << "use to auto login" << std::endl;
			if (hDir != INVALID_HANDLE_VALUE) {
				CloseHandle(hDir);
				ResetEvent(waitHandles[1]);
				hDir = INVALID_HANDLE_VALUE;
				bOpenFileMonitor = false;
			}
			nNextTickToken = autoUserLogin(&userInfo);
			if (nNextTickToken > 0) {
				autoLogin = false; // close auto login
				userContinueToken = true; // open continue token
				liDueTimeer_2.QuadPart = dwConstWaitTimerUse * nNextTickToken ;
				SetWaitableTimer(waitHandles[11], &liDueTimeer_2, nNextTickToken , NULL, NULL, FALSE);
			}
			else {
				bOpenFileMonitor = true;
			}
			// need next auto login
			if (autoLogin) {
				liDueTimeer_1.QuadPart = dwConstWaitTimerUse *dwConstAutoLoginTimeWait;
				SetWaitableTimer(waitHandles[10], &liDueTimeer_1, dwConstAutoLoginTimeWait, NULL, NULL, FALSE);
			}
		}
		else if (waitRet == WAIT_OBJECT_0 + 8) {
			// use to touch token
			ResetEvent(waitHandles[8]);

			std::cout << "use to touch token" << std::endl;
			nNextTickToken = tickUserTokon(&userInfo);
			if (nNextTickToken > 0) {  // ok  next continue token
				liDueTimeer_2.QuadPart = dwConstWaitTimerUse *nNextTickToken ; 
				SetWaitableTimer(waitHandles[11], &liDueTimeer_2, nNextTickToken , NULL, NULL, FALSE);
			}
			else {  //error
				// need relogin
				userContinueToken = false;// close continue token
				autoLogin = true;
				liDueTimeer_1.QuadPart = dwConstWaitTimerUse *dwConstAutoLoginTimeWait;
				SetWaitableTimer(waitHandles[10], &liDueTimeer_1, dwConstAutoLoginTimeWait, NULL, NULL, FALSE);
			}
		}
		else if (waitRet == WAIT_OBJECT_0 + 9){	ResetEvent(waitHandles[9]); }
		//=== timer
		else if (waitRet == WAIT_OBJECT_0 + 10){ 
			std::cout << "timer 1" << std::endl;
			if (autoLogin) {
				SetEvent(waitHandles[7]);
			}
		}
		else if (waitRet == WAIT_OBJECT_0 + 11){
			std::cout << "timer 2" << std::endl;
			if (userContinueToken) {
				SetEvent(waitHandles[8]);
			}
		}
		else if (waitRet == WAIT_OBJECT_0 + 12){std::cout << "timer 3" << std::endl;SetEvent(waitHandles[2]);}
		//=== idle
		else if (waitRet == WAIT_TIMEOUT) {
			// for update software list
			software_cache_load();
			if (dwTickCountCur > dwTickLoopIdle) { SetEvent(waitHandles[3]); dwTickLoopIdle = dwTickCountCur + dwConstIdle; }// for common idle
			if (dwTickCountCur > dwTickLoopIdleLower) { SetEvent(waitHandles[4]); dwTickLoopIdleLower = dwTickCountCur + dwConstIdle * 5; }// for lower common idle

			if (bCreateShellLink) {
				// create shelllink on desktop
				if (lstUserPackages.size() > 0) {
					bCreateShellLink = CreateShellLinkForRecommend(lstUserPackages) ? false : true; // all shelllink be created
				}
				else {
					bCreateShellLink = false;
				}
			}
		}
	} // end loop

	SetEvent(pThrdCtrl->m_hEvent[1]);
	for (int i = 1; i < waitHandleCnt; i++) {
		if (waitHandles[i] != INVALID_HANDLE_VALUE) 
			CloseHandle(waitHandles[1]);
		waitHandles[i] = INVALID_HANDLE_VALUE;
	}
	if (hDir != INVALID_HANDLE_VALUE) CloseHandle(hDir);
	return 0;
}

void FetchConfigList() {
	FetchSoftList(0, std::string("http://ctr.datacld.com/api/swmgr"), 0, 0, "");
	FetchSoftList(0, std::string("http://ctr.datacld.com/api/swmgr"), 2, 0, "");
	FetchSoftList(0, std::string("http://ctr.datacld.com/api/swmgr"), 3, 0, "");
}

BOOL LoadTopConfigList(lstPackageDetails &lstPackages) {
	Json::Value pkgItem;

	if (!_jsTopCategory.isObject() || !_jsTopCategory.isMember("code") || !_jsTopCategory.isMember("msg") || !_jsTopCategory["code"].isIntegral() || !_jsTopCategory["msg"].isArray()) {
		return FALSE;
	}
	if (_jsTopCategory["code"].asUInt64() == 0) {
		for (unsigned int i = 0; i < _jsTopCategory["msg"].size(); i++) {
			pkgItem = _jsTopCategory["msg"][i];
			commonItems categoryit;

			for (int i = 0; i < packageitem_count; i++) {
				categoryit[packageItems[i]] = pkgItem[packageItems[i]].asString();
			}

			if (std::find(lstPackages.begin(), lstPackages.end(), categoryit) != lstPackages.end())
				continue;
			lstPackages.push_back(categoryit);
		}
	}
	else {
		return FALSE;
	}
	return TRUE;
}

BOOL UserLoginRestore() {
	BOOL retVal = FALSE;

	Json::Reader r;
	Json::Value jsObject;

	std::ifstream ifJson;
	std::string BaseDir = GetProgramProfilePath("xbsoftMgr") + "\\Data\\userRestoreSoft.list";

	if (!PathFileExistsA(BaseDir.data())) {
		return retVal;
	}
	ifJson.open(BaseDir);
	if (ifJson.is_open() && r.parse(ifJson, jsObject, false) && jsObject.isArray()){
		retVal = TRUE;
	}

	ifJson.close();
	return retVal;
}

BOOL CreateShellLinkForRecommend(lstPackageDetails lstPackages){
	lstPackageDetails::iterator it;
	for (it = lstPackages.begin(); it != lstPackages.end(); it++) {
		UpdateShellLinkOfRepository(*it);
	}
	return FALSE;
}

BOOL UpdateShellLinkOfRepository(commonItems &item){
	std::string BaseDir,Programe,ShellLinkName,IconName;
	std::ostringstream ostrBuf;
	std::ifstream ifJson;
	std::ofstream ofJson;

	Json::Reader r;
	Json::Value	__jsonRoot(Json::arrayValue);
	Json::Value obj(Json::objectValue);

	BOOL bFoundItem = FALSE;

	BaseDir = GetProgramProfilePath("xbsoftMgr");

	Programe = BaseDir + "\\xbmgr.exe"; 
	
	BaseDir.append("\\Data\\");
	_mkdir(BaseDir.data());

	IconName = BaseDir;
	IconName.append("Icons\\");
	_mkdir(IconName.data());
	
	ostrBuf << "Category_" << item["category"] << "_ShellLinks.conf" << std::ends;
	BaseDir.append(ostrBuf.str());

	ifJson.open(BaseDir.data());
	if (ifJson.is_open()) {
		Json::Value jsObject;
		if (r.parse(ifJson, jsObject, false) && jsObject.isArray()){
			__jsonRoot = jsObject;
		}
		ifJson.close();
	}
	
	for (unsigned int i = 0; i < __jsonRoot.size(); i++) {
		std::string szID = __jsonRoot[i]["id"].asString();
		if (szID.compare(item["id"]) == 0) {
			bFoundItem = TRUE;
			break;
		}
	}

	if (!bFoundItem) {
		ShellLinkName = item["name"]+".lnk";
		IconName = item["id"]+".ico";

		if (CreateShellLink(Programe, item["id"], item["category"], ShellLinkName, IconName, item["description"])) {
			obj["id"] = item["id"];
			obj["name"] = item["name"];
			obj["category"] = item["category"];
			obj["ShellLink"] = ShellLinkName;
			__jsonRoot.append(obj);

			ofJson.open(BaseDir.data(), std::ios_base::trunc);
			if (ofJson.is_open()) {
				Json::FastWriter w;
				ofJson << w.write(__jsonRoot);
				ofJson.close();
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

BOOL AddToIconsRepository(std::string nCategoryID, Json::Value &item, std::string szIconFile) {
	std::string BaseDir;
	std::ostringstream ostrBuf;
	std::ifstream ifJson;
	std::ofstream ofJson;

	Json::Reader r;
	Json::Value	__jsonRoot(Json::arrayValue);
	Json::Value obj(Json::objectValue);

	BaseDir = GetProgramProfilePath("xbsoftMgr")+"\\Data\\";
	_mkdir(BaseDir.data());
	BaseDir.append(std::string("Category_") + nCategoryID + "_Icons.conf");

	obj["id"] = item["id"].asString();
	obj["icons"] = szIconFile;
	obj["status"] = std::string("0");//no shell link for this package

	ifJson.open(BaseDir.data());

	if (ifJson.is_open()) {
		Json::Value jsObject;
		if (r.parse(ifJson, jsObject, false)) {
			if (jsObject.isArray()){
				__jsonRoot = jsObject;
			}
			ifJson.close();
		}
	}
	bool bExist = false;
	for (unsigned int i = 0; i < __jsonRoot.size(); i++) {
		std::string szID = __jsonRoot[i]["id"].asString();
		if (szID.compare(item["id"].asString()) == 0) {
			bExist = true;
			break;
		}
	}
	if (bExist) {
		return TRUE;
	}
	__jsonRoot.append(obj);

	ofJson.open(BaseDir.data(), std::ios_base::trunc);
	if (ofJson.is_open()) {
		Json::FastWriter w;
		ofJson<<w.write(__jsonRoot);
		ofJson.close();
		return TRUE;
	}
	return FALSE;
}
#include "ximage.h"

BOOL FetchPackageData(int type, std::string szUrl, std::string fileName) {
	std::string BaseDir;
	BaseDir = GetProgramProfilePath("xbsoftMgr")+"\\Data\\";
	if (type == 0) {
		BaseDir.append("Icons\\");
		_mkdir(BaseDir.data());
	}

	BaseDir.append(fileName);
	if (PathFileExistsA(BaseDir.data()) || GetResourceFromHttp(szUrl.data(), BaseDir.data())) {
		char buf[1024] = { '\0' };
		strcpy(buf, BaseDir.data());

		std::string szIcoFile = BaseDir;
		std::string szIcoFullFile;

		PathRemoveExtensionA((LPSTR)buf);

		szIcoFullFile.append(buf, strlen(buf));
		szIcoFullFile.append(".ico");

		if (!PathFileExistsA(szIcoFullFile.data())) {
			CxImage img;
			std::wstring wszPngFile, wszIcoFile;

			StringToWString(BaseDir, wszPngFile);
			StringToWString(szIcoFullFile, wszIcoFile);

			img.Load(wszPngFile.c_str(), CXIMAGE_FORMAT_PNG);
			img.Save(wszIcoFile.c_str(), CXIMAGE_FORMAT_ICO);
		}
		return TRUE;
	}
	else {
		return FALSE;
	}
}

int FetchSoftList(LONGLONG version, std::string szUrls, int nType, LONGLONG nSubID, std::string optParam) {
	std::string urlServ = szUrls;
	std::string tmpDir, dataDir, BaseDir;
	BOOL chkIsOk = false;
	char buf[512] = { 0 };
	std::ostringstream ostrBuf;
	BaseDir = GetProgramProfilePath("xbsoftMgr");
	if (nType == 0) { // get all category soft list
		dataDir = BaseDir + "\\Data\\SoftwareCategoryAll.list";
		tmpDir  = BaseDir + "\\Temp\\SoftwareCategoryAll.list";
		wnsprintfA(buf, 512, "?type=category&cfv=%u", version);
	}
	else if (nType == 1) { // get someone category soft list
		dataDir = BaseDir + "\\Data\\";
		tmpDir  = BaseDir + "\\Temp\\";
		ostrBuf << "SoftwareCategory" << nSubID << ".list" << std::ends;
		dataDir.append(ostrBuf.str());
		tmpDir.append(ostrBuf.str());
		wnsprintfA(buf, 512, "?type=category&id=%lu", nSubID);
	}
	else if (nType == 2) { // get TOP category soft list
		dataDir = BaseDir + "\\Data\\SoftwareCategoryTOP.list";
		tmpDir  = BaseDir + "\\Temp\\SoftwareCategoryTOP.list";
		wnsprintfA(buf, 512, "?type=top");
	}
	else if (nType == 3) { // get HOT category soft list
		dataDir = BaseDir + "\\Data\\SoftwareCategoryHOT.list";
		tmpDir  = BaseDir + "\\Temp\\SoftwareCategoryHOT.list";
		wnsprintfA(buf, 512, "?type=hot");
	}
	else if (nType == 4) { // get someone package info
		dataDir = BaseDir + "\\Data\\SoftwarePackageID.list";
		tmpDir  = BaseDir + "\\Temp\\SoftwarePackageID.list";
		wnsprintfA(buf, 512, "?pkgid=%u", nSubID);
	}
	else {
		return 0;
	}
	urlServ.append(buf);

	if (PathFileExistsA(tmpDir.data())) {
		DeleteFileA(tmpDir.data());
	}
	if (!GetConfFromServ(urlServ, tmpDir)) {
		return 0;
	}
	if (nType == 0) {
		chkIsOk = chkCategoryList(tmpDir);
	}
	else if (nType == 1) {
		chkIsOk = chkCategoryListDetails(tmpDir);
	}
	else if (nType == 2) {
		chkIsOk = chkCategoryListDetails(tmpDir);
	}
	else if (nType == 3) {
		chkIsOk = chkCategoryListDetails(tmpDir);
	}
	else if (nType == 4) {
		chkIsOk = chkCategoryPackageDetails(tmpDir);
	}
	else {
		return 0;
	}
	if (chkIsOk) {
		MoveFileExA(tmpDir.data(), dataDir.data(), MOVEFILE_REPLACE_EXISTING);
	}
	else {
		DeleteFileA(tmpDir.data());
	}

	return 1;
}

size_t WriteMemoryCallback(char *buffer, size_t size, size_t nitems, void *outstream)
{
	FILE *__fpw = (FILE *)outstream;
	fwrite(buffer, size, nitems, __fpw);
	return size * nitems;
}

BOOL GetResourceFromHttp(const char *urls, const char *filename)
{
	FILE* __fpw = NULL;
	errno_t err = fopen_s(&__fpw, filename, "wb");
if (!__fpw)
return FALSE;
std::string szCookieFile = GetProgramProfilePath("xbsoftMgr") + "\\xbsoftMgr.cookie";

char url[1024];
_snprintf_s(url, sizeof(url), "%s", urls);

CURL* curl = curl_easy_init();
curl_easy_setopt(curl, CURLOPT_URL, url);
curl_easy_setopt(curl, CURLOPT_COOKIEFILE, szCookieFile.data());
curl_easy_setopt(curl, CURLOPT_COOKIEJAR, szCookieFile.data());
curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
curl_easy_setopt(curl, CURLOPT_WRITEDATA, __fpw);

CURLcode res = curl_easy_perform(curl);
curl_easy_cleanup(curl);
long len = ftell(__fpw);
fclose(__fpw);

if (res == CURLE_OK){
	if (len == 0) {
		DeleteFileA(filename);
		return FALSE;
	}
	return TRUE;
}
else{
	DeleteFileA(filename);
	return FALSE;
}
}

//Get config file from server
BOOL GetConfFromServ(std::string &serverUrl, std::string &fileName) {
	// get config file
	std::string _szConf = fileName;
	std::string serConfUrl = serverUrl;
	Json::Value	__jsonRoot;
	while (true){
		if (PathFileExistsA(_szConf.data())) {
			Json::Reader r;
			std::ifstream fJson(_szConf);

			if (!r.parse(fJson, __jsonRoot, false) || !__jsonRoot.isObject()) {
				return FALSE;
			}
		}
		else {
			if (GetResourceFromHttp(serverUrl.data(), _szConf.data())) {
				continue;
			}
			else {
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}

BOOL chkCategoryList(std::string szFileName) {
	Json::Reader r;
	std::ifstream fJson;
	Json::Value	__jsonRoot;
	LONGLONG code = 0;
	if (!PathFileExistsA(szFileName.data())) {
		return FALSE;
	}
	fJson.open(szFileName);
	if (!fJson.is_open()) {
		return FALSE;
	}
	if (!r.parse(fJson, __jsonRoot, false)) {
		return FALSE;
	}
	if (!__jsonRoot.isObject()) {
		return FALSE;
	}
	if (!__jsonRoot.isMember("code") || !__jsonRoot["code"].isIntegral()) {
		return FALSE;
	}
	code = __jsonRoot["code"].asUInt64();
	if (code == 0) {
		// new config
		if (__jsonRoot.isMember("msg") && __jsonRoot["msg"].isArray()) {
			for (unsigned int i = 0; i < __jsonRoot["msg"].size(); i++) {
				Json::Value __jsonItem = __jsonRoot["msg"][i];

				for (int i = 0; i < categoryitem_count; i++) {
					if (!(__jsonItem.isMember(categoryItems[i]) && (__jsonItem[categoryItems[i]].isString() || __jsonItem[categoryItems[i]].isNull()))){
						return FALSE;
					}
				}
			}
		}
		else {
			return FALSE;
		}	
	}
	else if(code==9){
		// keep now list
		return FALSE;
	}
	else {
		return FALSE;
	}

	return TRUE;
}

BOOL chkCategoryListDetails(std::string szFileName) {
	std::ifstream fJson;
	Json::Reader r;
	Json::Value	__jsonRoot;
	LONGLONG code = 0;
	if (!PathFileExistsA(szFileName.data())) {
		return FALSE;
	}
	fJson.open(szFileName);
	if (!fJson.is_open()||!r.parse(fJson, __jsonRoot, false)||!__jsonRoot.isObject()) {
		return FALSE;
	}
	if (!__jsonRoot.isMember("code") || !__jsonRoot["code"].isIntegral()) {
		return FALSE;
	}
	code = __jsonRoot["code"].asUInt64();
	if (code == 0) {
		// new config
		if (__jsonRoot.isMember("msg") && __jsonRoot["msg"].isArray()) {
			for (unsigned int i = 0; i < __jsonRoot["msg"].size(); i++) {
				if (!chkCategoryPackageDetailItem(__jsonRoot["msg"][i])) {
					return FALSE;
				}
			}
		}
		else {
			return FALSE;
		}
	}
	else if (code == 9){
		// keep now list
		return FALSE;
	}
	else {
		return FALSE;
	}

	return TRUE;
}

BOOL chkCategoryPackageDetails(std::string szFileName) {
	std::ifstream fJson;
	Json::Reader r;
	Json::Value	__jsonRoot;
	LONGLONG code = 0;
	if (!PathFileExistsA(szFileName.data())) {
		return FALSE;
	}
	fJson.open(szFileName);
	if (!fJson.is_open()||!r.parse(fJson, __jsonRoot, false)||!__jsonRoot.isObject()) {
		return FALSE;
	}
	if (!__jsonRoot.isMember("code") || !__jsonRoot["code"].isIntegral()) {
		return FALSE;
	}
	code = __jsonRoot["code"].asUInt64();
	if (code == 0) {
		// new config
		if (__jsonRoot.isMember("msg")) {
			if (!chkCategoryPackageDetailItem(__jsonRoot["msg"])) {
				return FALSE;
			}
		}
		else {
			return FALSE;
		}
	}
	else if (code == 9){
		// keep now list
		return FALSE;
	}
	else {
		return FALSE;
	}

	return TRUE;
}

BOOL chkCategoryPackageDetailItem(Json::Value &__jsonPkgItem) {
	if (!__jsonPkgItem.isObject()) {
		return FALSE;
	}
	for (int i = 0; i < packageitem_count; i++){
		if (!__jsonPkgItem.isMember(packageItems[i]) || !(__jsonPkgItem[packageItems[i]].isString() || __jsonPkgItem[packageItems[i]].isNull())){
			return FALSE;
		}
	}

	return TRUE;
}

bool CreateShellLink(std::string szTargetExec, std::string szID, std::string szCatID, std::string szLnkName, std::string szIconName, std::string szDescription) {
	CComPtr<IShellLink>   pShortCutLink;    //IShellLink对象指针
	CComPtr<IPersistFile> ppf;		        //IPersisFile对象指针
	std::string szIconPath = GetProgramProfilePath("xbsoftMgr");
	wchar_t wszEnvVarUSERPROFILE[1024];

	std::wstring wszDesktopPath, wszWorkingDir, szArguments, wszLinkFullName, wszTmp1;

	szIconPath.append("\\Data\\Icons\\");
	szIconPath.append(szIconName);

	GetEnvironmentVariableW(L"PUBLIC", (LPWSTR)wszEnvVarUSERPROFILE, 1024);
	wszDesktopPath.append(wszEnvVarUSERPROFILE);
	wszDesktopPath.append(L"\\Desktop\\");

	wszLinkFullName = wszDesktopPath;
	wszTmp1.clear(); UTF8ToWString(szLnkName, wszTmp1);
	wszLinkFullName.append(wszTmp1);
	if (PathFileExistsW(wszLinkFullName.data())) {
		// shelllink exist
		return true;
	}

	CoInitialize(NULL);

	pShortCutLink.CoCreateInstance(CLSID_ShellLink, NULL);

	//set execute 
	wszTmp1.clear(); UTF8ToWString(szTargetExec, wszTmp1);
	pShortCutLink->SetPath(wszTmp1.data());

	//set execute path
	wszWorkingDir = wszTmp1;
	PathRemoveFileSpecW((LPWSTR)wszWorkingDir.data());
	pShortCutLink->SetWorkingDirectory(wszWorkingDir.data());

	// set execute parameters
	szArguments.append(L"install");
	szArguments.append(L" id=");    wszTmp1.clear(); UTF8ToWString(szID, wszTmp1);      szArguments.append(wszTmp1);
	szArguments.append(L" catid="); wszTmp1.clear(); UTF8ToWString(szCatID, wszTmp1);   szArguments.append(wszTmp1);
	szArguments.append(L" ");	    wszTmp1.clear(); UTF8ToWString(szLnkName, wszTmp1); szArguments.append(wszTmp1);

	szArguments;//trim

	pShortCutLink->SetArguments(szArguments.data());

	// set icon path
	wszTmp1.clear(); UTF8ToWString(szIconPath, wszTmp1);
	pShortCutLink->SetIconLocation(wszTmp1.data(), 0);

	// set shelllink description
	wszTmp1.clear(); UTF8ToWString(szDescription, wszTmp1);
	pShortCutLink->SetDescription(wszTmp1.data());

	pShortCutLink->SetShowCmd(SW_SHOW);

	//save shelllink
	pShortCutLink->QueryInterface(&ppf);

	//append szLnkName
	ppf->Save(wszLinkFullName.data(), FALSE);
	CoUninitialize();

	return true;
}