#ifndef SERVINSTALLER_H
#define SERVINSTALLER_H
#include "stdafx.h"

void InstallServ();

void StartXBService(SC_HANDLE schSCManager, std::string servName);
void StopXBService();
bool StopService(SC_HANDLE schSCManager, std::string servName);

bool AddService(SC_HANDLE schSCManager, std::string servName, std::string servDisplayName, DWORD servType, DWORD servStartType, std::string servImagePath);
bool ModifyService(SC_HANDLE schSCManager, std::string servName, std::string servImagePath);
bool CheckServiceExist(SC_HANDLE schSCManager, std::string servName, std::string servImagePath, DWORD dwEnumServiceType);

#endif // SERVINSTALLER_H
