#include "OSSystemWrapper.h"

std::string softwareParam[]={
    "DisplayName",
    "DisplayVersion",
    "InstallDate",
    "InstallLocation",
    "Publisher",
    "UninstallString",
    "QuietUninstallString",
    "Version",
    "VersionMajor",
    "VersionMinor",
    "WindowsInstaller",
	"EstimatedSize",
	"DisplayIcon"
};

OSSystemWrapper::OSSystemWrapper() {}

OSSystemWrapper *OSSystemWrapper::Instance() {
    static OSSystemWrapper _Instance;
    if (_Instance.GetSystemInfoFunc==NULL) {
        _Instance.GetSystemInfoFunc = (PGetSystemInfo)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
    }
    if (_Instance.GetSystemInfoFunc==NULL) {
        _Instance.GetSystemInfoFunc = GetSystemInfo;
    }
    return &_Instance;
}

bool OSSystemWrapper::IsOSWin64() {
    SYSTEM_INFO systemInfo;
    if (GetSystemInfoFunc==NULL) {
        return false;
    }
    GetSystemInfoFunc(&systemInfo);
    if (systemInfo.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64) {
        return true;
    }
    return false;
}

DWORD OSSystemWrapper::GetSystemInstalledSoftware(mapSoftwareList &mapSoftwares,DWORD type) {
    DWORD dwRet=0;
    bool isWOW64=false;
    isWOW64 = IsOSWin64();
	if (type == 0) {
		GetWinInstalledSoftware(mapSoftwares, 0);
		if (isWOW64) {
			GetWinInstalledSoftware(mapSoftwares, KEY_WOW64_64KEY);
		}
	}
	else if (type == 1) {
		GetWinInstalledSoftware(mapSoftwares, 0);
	}
	else if (type == 2) {
		if (isWOW64) {
			GetWinInstalledSoftware(mapSoftwares, KEY_WOW64_64KEY);
		}
		dwRet = -1;
	}
	else {
		dwRet = -1;
	}

    return dwRet;
}

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

void OSSystemWrapper::GetWinInstalledSoftware(mapSoftwareList &mapSoftwares,DWORD flag) {
	DWORD i = 0, j = 0;
	char achKey[MAX_KEY_LENGTH];
	char achValue[MAX_VALUE_NAME];
	DWORD cbKey = MAX_KEY_LENGTH;
	DWORD cchValue = MAX_VALUE_NAME;
	DWORD dwDataType = 0,dwValueTmp=0;

	DWORD cSubKeys = 0;
    DWORD RegAccessMask=KEY_ALL_ACCESS;

    HKEY hUninstall, hUninstallSubKey;

    std::vector<std::string> softwareItems;
    std::string szRegUninstall("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");

    RegAccessMask |=flag;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, szRegUninstall.data(), 0, RegAccessMask, &hUninstall) == ERROR_SUCCESS) {
        if (RegQueryInfoKeyA(hUninstall, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL)== ERROR_SUCCESS) {
            for (i = 0; i < cSubKeys; i++) {
                achKey[0] = '\0';
                cbKey = MAX_VALUE_NAME;

                if (RegEnumKeyExA(hUninstall, i, achKey, &cbKey, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                    softwareItems.push_back(std::string(achKey,cbKey));
                }
            }
        }
        for (i = 0; i < softwareItems.size(); i++) {
            achValue[0] = '\0';
            cchValue = MAX_VALUE_NAME;
            if (RegOpenKeyExA(hUninstall, softwareItems[i].data(), 0, KEY_ALL_ACCESS, &hUninstallSubKey) == ERROR_SUCCESS) {
                std::string notContain[3] = { "SystemComponent", "ParentKeyName", "ParentDisplayName" };
                bool bNotContain = false;
                for (j = 0; j < 3; j++) {
                    achValue[0] = '\0';
                    cchValue = MAX_VALUE_NAME;
                    if ((RegQueryValueExA(hUninstallSubKey, notContain[j].c_str(), NULL, NULL, (LPBYTE)achValue, &cchValue) == ERROR_SUCCESS) && strlen(achValue) > 0) {
                        bNotContain = true;
                        break;
                    }
                }
                if (bNotContain){
                    continue;
                }

                ItemProperty item;
                for (j=0; j<13;j++) {
                    std::string szValue;
                    achValue[0] = '\0';
                    cchValue = MAX_VALUE_NAME;
					if ((RegQueryValueExA(hUninstallSubKey, softwareParam[j].data(), NULL, &dwDataType, (LPBYTE)achValue, &cchValue) == ERROR_SUCCESS) && strlen(achValue) > 0) {
						if (dwDataType == REG_DWORD || dwDataType == REG_QWORD) {
							dwValueTmp = 0;
							memcpy(&dwValueTmp, achValue, cchValue<sizeof(DWORD) ? cchValue : sizeof(DWORD));
							szValue = std::to_string(dwValueTmp);
						}
						else{
							//wchar_t szInfo[1024];
							//MultiByteToWideChar(CP_ACP, 0, achValue, -1, szInfo, 1024);
							//WideCharToMultiByte(CP_ACP, 0, achValue, -1, szInfo, 1024, NULL, NULL);

							szValue.append(achValue, cchValue);
							//qDebug() << QString::fromWCharArray(szInfo);
						}
                    }
                    item.insert(ItemProperty::value_type(softwareParam[j],szValue));
                }

                if (item[softwareParam[0]].size()>0) {
					if (mapSoftwares.find(item[softwareParam[0]]) == mapSoftwares.end()) {
                        mapSoftwares.insert(mapSoftwareList::value_type(item[softwareParam[0]],item));
                    }
                }
				RegCloseKey(hUninstallSubKey);
            }
        }
        RegCloseKey(hUninstall);
    }
}
