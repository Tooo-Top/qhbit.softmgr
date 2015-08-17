#pragma once
#include "stdafx.h"

class CCurlEnv
{
protected:
	CCurlEnv();
	~CCurlEnv();
protected:
	BOOL m_bStatus;
public:
	static CCurlEnv *Instance();
public:
	BOOL GetStatus();
};

