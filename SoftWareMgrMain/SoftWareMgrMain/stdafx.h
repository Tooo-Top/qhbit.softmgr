// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#define WIN32_LEAN_AND_MEAN
// Windows Headers
#include <SDKDDKVer.h>
#include <Windows.h>

#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include <Wincrypt.h>
#pragma comment(lib,"Advapi32.lib")
#include <Wtsapi32.h>
#pragma comment ( lib, "Wtsapi32.lib" )
#include <Userenv.h>
#pragma comment ( lib, "Userenv.lib" )
#include <tlhelp32.h>

#pragma comment ( lib, "Wldap32.lib" )
#pragma comment ( lib, "Ws2_32.lib" )

//#include <Wbemidl.h>
//#pragma comment ( lib, "wbemuuid.lib" )

#include <assert.h>
#include <direct.h>
#include <intsafe.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <list>
#include <map>
#include <functional>
#include <algorithm>

//#include <strsafe.h>

#include <comdef.h>
#include <comutil.h>
#include <atlbase.h>
#include <atlcomcli.h>
#include <Shobjidl.h>



// TODO:  在此处引用程序需要的其他头文件
