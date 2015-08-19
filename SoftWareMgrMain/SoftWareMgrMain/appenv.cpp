#include "appenv.h"

std::wstring _ServeName = L"XB SoftwareManager";
std::wstring wszEvtName = L"{BC4C831E-229B-462A-9272-1EF5E1652CC9}";

std::string GetModulePath(HMODULE hModule){
    char buf[1024] = {'\0'};
    std::string strDir;

    ::GetModuleFileNameA( hModule, buf, MAX_PATH);
    PathRemoveFileSpecA(buf);
    strDir.append(buf);
    return strDir;
}

std::string GetAppdataPath(std::string szCompany) {
    char buf[1024] = {'\0'};
    std::string szAppdataPath;
    if (GetEnvironmentVariableA("CommonProgramFiles",buf,1024)==0) {
        szAppdataPath = GetModulePath(NULL);
    }
    else {
        szAppdataPath.append(buf);
    }

    szAppdataPath.append("\\");
    szAppdataPath.append(szCompany);
    _mkdir(szAppdataPath.data());
    return szAppdataPath;
}

std::string GetProgramProfilePath(std::string name) {
    std::string szProgProfile;
    szProgProfile = GetAppdataPath();
    szProgProfile.append("\\");
    szProgProfile.append(name);
    _mkdir(szProgProfile.data());
    return szProgProfile;
}

std::string GetFilePathFromFile(std::string szFile) {
    std::string strDir;
    char buf[1024] = {'\0'};

    strcpy_s(buf,szFile.data());
    PathRemoveFileSpecA(buf);
    strDir.append(buf);
    return strDir;
}

//=======================================

void InitDir(){
	std::string tmpDir;
	std::string BaseDir = GetProgramProfilePath("xbsoftMgr");

    // ----------------------------------
    tmpDir = BaseDir+"\\UpdateDir" ;_mkdir(tmpDir.data());
    // ----------------------------------
    tmpDir = BaseDir+"\\Data" ;	    _mkdir(tmpDir.data());
    // ----------------------------------
    tmpDir = BaseDir+"\\Conf" ;	    _mkdir(tmpDir.data());
    // ----------------------------------
    tmpDir = BaseDir+"\\Temp" ;	    _mkdir(tmpDir.data());
    // ----------------------------------
}

BOOL StringToWString(const std::string &str, std::wstring &wstr)
{
	int nLen = (int)str.length();
	wstr.resize(nLen, L' ');

	int nResult = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str.c_str(), nLen, (LPWSTR)wstr.c_str(), nLen);

	if (nResult == 0)
	{
		return FALSE;
	}

	return TRUE;
}
//wstring高字节不为0，返回FALSE
BOOL WStringToString(const std::wstring &wstr, std::string &str)
{
	int nLen = (int)wstr.length();
	str.resize(nLen, ' ');

	int nResult = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wstr.c_str(), nLen, (LPSTR)str.c_str(), nLen, NULL, NULL);

	if (nResult == 0)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL UTF8ToWString(const std::string &str, std::wstring &wstr)
{
	int nLen = (int)str.size(); 
	nLen = (int)WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)wstr.c_str(), nLen, NULL, 0, NULL, NULL);
	wstr.resize(nLen, L' ');

	int nResult = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str.c_str(), nLen, (LPWSTR)wstr.c_str(), nLen);

	if (nResult == 0) {
		return FALSE;
	}

	return TRUE;
}
//wstring高字节不为0，返回FALSE
BOOL WStringToUTF8(const std::wstring &wstr, std::string &str)
{
	int nLen = (int)wstr.length();

	str.resize(nLen, ' ');

	int nResult = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wstr.c_str(), nLen, (LPSTR)str.c_str(), nLen, NULL, NULL);

	if (nResult == 0)
	{
		return FALSE;
	}

	return TRUE;
}
