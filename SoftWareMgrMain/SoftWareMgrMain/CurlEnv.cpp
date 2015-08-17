#include "CurlEnv.h"
#include "curl\curl.h"

CCurlEnv::CCurlEnv() : m_bStatus(FALSE)
{
}


CCurlEnv::~CCurlEnv()
{
	if (m_bStatus) {
		::curl_global_cleanup();
	}
}

CCurlEnv *CCurlEnv::Instance() {
	static CCurlEnv _instance;
	if (!_instance.m_bStatus) {
		if (::curl_global_init(CURL_GLOBAL_WIN32) == CURLE_OK)
			_instance.m_bStatus = TRUE;

	}
	return &_instance;
}

BOOL CCurlEnv::GetStatus(){
	return m_bStatus;
}