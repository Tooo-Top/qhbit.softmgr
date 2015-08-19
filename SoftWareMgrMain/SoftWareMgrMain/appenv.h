#ifndef APPENV_H
#define APPENV_H
#include "stdafx.h"

extern std::wstring _ServeName;
extern std::wstring wszEvtName;

std::string GetModulePath(HMODULE hModule = NULL);
std::string GetAppdataPath(std::string szCompany="HurricaneTeam");
std::string GetProgramProfilePath(std::string name);
std::string GetFilePathFromFile(std::string szFile);

// ==========================================
void InitDir();

BOOL StringToWString(const std::string &str, std::wstring &wstr);
BOOL WStringToString(const std::wstring &wstr, std::string &str);
BOOL UTF8ToWString(const std::string &str, std::wstring &wstr);

#endif // APPENV_H
