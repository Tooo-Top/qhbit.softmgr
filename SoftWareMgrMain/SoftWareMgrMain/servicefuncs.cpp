#include "appenv.h"
#include "servicefuncs.h"
#include "SysReportLog.h"
#include "ServMain.h"

void RunServiceEntry() {
    SERVICE_TABLE_ENTRYW DispatchTable[] = {
        { (LPWSTR)_ServeName.data(), (LPSERVICE_MAIN_FUNCTIONW) CServMain::_ServiceMain },
        { NULL, NULL }
    };

    StartServiceCtrlDispatcherW(DispatchTable);
}

