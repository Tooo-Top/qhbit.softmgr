#include "stdafx.h"
#include "FuncsImp.h"
#include "appenv.h"

typedef struct _redoFetchIcon {
	std::string id;
	std::string url;
} RedoFetchIcon;

Json::Value _jsSoftCategory;
Json::Value _jsTopCategory;
Json::Value _jsHotCategory;
Json::Value _jsCurPackageList;
//for "allcategory,top,hot" -- read or download
std::list<std::string> FirstTask;
std::list<std::string> FirstTaskFailed;

//for "category detail"-- download
std::list<std::string> SecondTask;
std::list<std::string> SecondTaskFailed;

//for "package detail" -- icons
std::list<std::string> ThirdTask;
//// _jsCurPackageList,itCurPackageIcon

//for icons
Json::ArrayIndex itTopIcon;
Json::ArrayIndex itHotIcon;
Json::ArrayIndex itCurPackageIcon;
std::list<Json::ArrayIndex> topIconsFailed;
std::list<Json::ArrayIndex> hotIconsFailed;
std::list<RedoFetchIcon> packageIconsFailed;

void software_cache_init(int refresh) {
	_jsSoftCategory.clear();
	_jsTopCategory.clear();
	_jsHotCategory.clear();
	_jsCurPackageList.clear();
	_jsSoftCategory = Json::Value();
	_jsTopCategory = Json::Value();
	_jsHotCategory = Json::Value();
	_jsCurPackageList = Json::Value();

	FirstTask.clear();
	FirstTaskFailed.clear();

	SecondTask.clear();
	SecondTaskFailed.clear();

	ThirdTask.clear();

	topIconsFailed.clear();
	hotIconsFailed.clear();
	packageIconsFailed.clear();
	if (refresh == 0) {
		FirstTask.push_back("all");
		FirstTask.push_back("top");
		FirstTask.push_back("hot");
	}
	else{
		FirstTaskFailed.push_back("all");
		FirstTaskFailed.push_back("top");
		FirstTaskFailed.push_back("hot");
	}
	itTopIcon = Json::ArrayIndex(-1);
	itHotIcon = Json::ArrayIndex(-1);
	itCurPackageIcon = Json::ArrayIndex(-1);
}

void software_cache_load() {
	std::string BaseDir, dataDir;
	std::ifstream fJson;
	Json::Reader r;
	Json::Value *curValue;

	BaseDir = GetProgramProfilePath("xbsoftMgr");

	std::string szTask;
	if (FirstTask.size() > 0) {
		szTask = FirstTask.front();
		FirstTask.pop_front();

		if (szTask.compare("all") == 0) {
			dataDir = BaseDir + "\\Data\\SoftwareCategoryAll.list";
			curValue = &_jsSoftCategory;
		}
		else if (szTask.compare("top") == 0) {
			dataDir = BaseDir + "\\Data\\SoftwareCategoryTOP.list";
			curValue = &_jsTopCategory;
		}
		else if (szTask.compare("hot") == 0) {
			dataDir = BaseDir + "\\Data\\SoftwareCategoryHOT.list";
			curValue = &_jsHotCategory;
		}
		else {
			return;
		}
		if (PathFileExistsA(dataDir.data())) {
			std::cout << "load file:" << dataDir << std::endl;
			curValue->clear();
			fJson.open(dataDir);
			if (!fJson.is_open() || !r.parse(fJson, *curValue, false) || !curValue->isObject()) {
				curValue->clear();
				DeleteFileA(dataDir.data());
//				FirstTaskFailed.push_back(szTask);
				return;
			}
			fJson.close();
			if (!curValue->isMember("code") || !curValue->isMember("msg") || !curValue->operator []("code").isIntegral() || !curValue->operator []("msg").isArray()) {
				curValue->clear();
				DeleteFileA(dataDir.data());
//				FirstTaskFailed.push_back(szTask);
				return;
			}
			if (curValue->operator []("code").asUInt64() == 9) {return;}
			if (curValue->operator []("code").asUInt64() == 0) {
				if (szTask.compare("all") == 0) {
					// start new one level task
					//FirstTaskFailed.push_back("top");
					//FirstTaskFailed.push_back("hot");
					//FirstTask.push_back("top");
					//FirstTask.push_back("hot");
					//add every category task
					for (Json::Value::iterator it = curValue->operator []("msg").begin(); it != curValue->operator []("msg").end(); it++){
						SecondTask.push_back((*it)["id"].asString());
					}
				}
				else if (szTask.compare("top") == 0) {
					itTopIcon = curValue->operator []("msg").empty() ? Json::ArrayIndex(-1) : Json::ArrayIndex(0);
				}
				else if (szTask.compare("hot") == 0) {
					itHotIcon = curValue->operator []("msg").empty() ? Json::ArrayIndex(-1) : Json::ArrayIndex(0);
				}
				return;
			}
			else {
				curValue->clear();
//				FirstTaskFailed.push_back(szTask);
				return;
			}
		}
		else {
			FirstTaskFailed.push_back(szTask);
			return;
		}
	}
	if (SecondTask.size() > 0) {
		szTask = SecondTask.front();
		SecondTask.pop_front();
		dataDir.clear();
		if (szTask.size() == 0) { return; }

		dataDir = BaseDir + "\\Data\\SoftwareCategory" + szTask + ".list";
		if (PathFileExistsA(dataDir.data())) { 
			std::cout << "load file:" << dataDir << std::endl; 
			ThirdTask.push_back(szTask);
			return; 
		}
		else {
			SecondTaskFailed.push_back(szTask);
			return;
		}
	}
}

void do_FirstTask()
{
	std::string szTask;
	szTask = FirstTaskFailed.front();
	FirstTaskFailed.pop_front();
	//fetch
	if (szTask.compare("all") == 0) {
		FetchSoftList(0, std::string("http://ctr.datacld.com/api/swmgr"), 0, 0, "");
	}
	else if (szTask.compare("top") == 0) {
		FetchSoftList(0, std::string("http://ctr.datacld.com/api/swmgr"), 2, 0, "");
	}
	else if (szTask.compare("hot") == 0) {
		FetchSoftList(0, std::string("http://ctr.datacld.com/api/swmgr"), 3, 0, "");
	}
	std::cout << "task:" << szTask << "downloaded" << std::endl;
	FirstTask.push_back(szTask);
}

void topORhotIcons(Json::ArrayIndex &aiItIcon, Json::Value &category, std::list<Json::ArrayIndex> &lstIconsFailed) {
	int times = 10;
	std::string iconFileName;
	while (aiItIcon < category["msg"].size() && times >= 0) {
		iconFileName = category["msg"][aiItIcon]["id"].asString();
		iconFileName.append(".png");

		// fetch;
		if (!FetchPackageData(0, category["msg"][aiItIcon]["iconUrl"].asString(), iconFileName)) {
			//add to icon list
			AddToIconsRepository(category["msg"][aiItIcon]["id"].asString(), category["msg"][aiItIcon], iconFileName);
		}
		else {
			lstIconsFailed.push_back(aiItIcon);
		}
		aiItIcon++;
		times--;
	}
	if (aiItIcon >= category["msg"].size()) {
		aiItIcon = Json::ArrayIndex(-1);
	}
}

void software_cache_idle() {
	std::string BaseDir, dataDir;
	std::ifstream fJson;
	Json::Reader r;

	LONGLONG code = 0;

	BaseDir = GetProgramProfilePath("xbsoftMgr");

	std::string szTask;

	// one level task finish,now resume failed task.
	if (FirstTask.size()==0 && FirstTaskFailed.size() > 0) {
		do_FirstTask();
	}
	if (FirstTask.size() > 0 || FirstTaskFailed.size()>0) { return; }

	//init top icons
	if (itTopIcon != Json::ArrayIndex(-1) && std::find(FirstTaskFailed.begin(), FirstTaskFailed.end(), "top") == FirstTaskFailed.end()) {
		topORhotIcons(itTopIcon, _jsTopCategory, topIconsFailed);
	}
	if (itHotIcon != Json::ArrayIndex(-1) && std::find(FirstTaskFailed.begin(), FirstTaskFailed.end(), "hot") == FirstTaskFailed.end()) {
		topORhotIcons(itHotIcon, _jsHotCategory, hotIconsFailed);
	}
	if (SecondTask.size() == 0 && SecondTaskFailed.size() > 0) {
		szTask = SecondTaskFailed.front();
		SecondTaskFailed.pop_front();
		//fetch
		int nSub = std::stoi(szTask);
		FetchSoftList(0, std::string("http://ctr.datacld.com/api/swmgr"), 1, nSub, "");
		SecondTask.push_back(szTask);
	}
	if (SecondTask.size() == 0 && SecondTaskFailed.size() > 0) { return ; }
}

void software_cache_idle_lower() {
	Json::Reader r;
	std::ifstream fJson;
	std::string BaseDir, dataDir, szTask, iconFileName;;
	int times = 10;

	BaseDir = GetProgramProfilePath("xbsoftMgr");
	if (_jsCurPackageList.isNull() && ThirdTask.size() > 0) {
		szTask = ThirdTask.front();
		ThirdTask.pop_front();
		dataDir = BaseDir + "\\Data\\SoftwareCategory" + szTask + ".list";
		if (PathFileExistsA(dataDir.data())) {
			fJson.open(dataDir);
			if (!fJson.is_open() || !r.parse(fJson, _jsCurPackageList, false) || !_jsCurPackageList.isObject()) {
				_jsCurPackageList.clear();
				_jsCurPackageList = Json::Value();
				DeleteFileA(dataDir.data());
//				SecondTaskFailed.push_back(szTask);
				return;
			}
			fJson.close();
			if (!_jsCurPackageList.isMember("code") || !_jsCurPackageList.isMember("msg") || !_jsCurPackageList["code"].isIntegral() || !_jsCurPackageList["msg"].isArray()) {
				_jsCurPackageList.clear();
				_jsCurPackageList = Json::Value();
				DeleteFileA(dataDir.data());
//				SecondTaskFailed.push_back(szTask);
				return;
			}
			if (_jsCurPackageList["code"].asUInt64() == 9) { return; }
			if (_jsCurPackageList["code"].asUInt64() == 0 ) {
				itCurPackageIcon = _jsCurPackageList["msg"].empty() ? Json::ArrayIndex(-1) : Json::ArrayIndex(0);
				return;
			}
			else {
				_jsCurPackageList.clear();
				_jsCurPackageList = Json::Value();
				DeleteFileA(dataDir.data());
//				SecondTaskFailed.push_back(szTask);
				return;
			}
		}
		else {
			SecondTaskFailed.push_back(szTask);
			return;
		}
	}
	else if(_jsCurPackageList.isObject()) {
		int times = 5;
		std::string iconFileName;
		while (itCurPackageIcon < _jsCurPackageList["msg"].size() && times >= 0) {
			RedoFetchIcon redoTask;
			redoTask.id = _jsCurPackageList["msg"][itCurPackageIcon]["id"].asString();
			redoTask.url = _jsCurPackageList["msg"][itCurPackageIcon]["iconUrl"].asString();

			iconFileName = redoTask.id;
			iconFileName.append(".png");

			std::cout << "down icon:id=" << redoTask.id << "--url:" << redoTask.url << std::endl;
			// fetch;
			if (FetchPackageData(0, _jsCurPackageList["msg"][itCurPackageIcon]["iconUrl"].asString(), iconFileName)) {
				//add to icon list
				AddToIconsRepository(_jsCurPackageList["msg"][itCurPackageIcon]["id"].asString(), _jsCurPackageList["msg"][itCurPackageIcon], iconFileName);
			}
			else {
				packageIconsFailed.push_back(redoTask);
			}
			itCurPackageIcon++;
			times--;
		}
		if (itCurPackageIcon >= _jsCurPackageList["msg"].size()) {
			_jsCurPackageList.clear();
			_jsCurPackageList = Json::Value();
			itCurPackageIcon = Json::ArrayIndex(-1);
		}
	}
	else if (packageIconsFailed.size()>0) {
		while (times > 0 && packageIconsFailed.size()>0) {
			RedoFetchIcon redoTask = packageIconsFailed.front();
			packageIconsFailed.pop_front();

			iconFileName = redoTask.id;
			iconFileName.append(".png");
			std::cout << "re down icon:id=" << redoTask.id << "--url:" << redoTask.url << std::endl;

			// fetch;
			if (!FetchPackageData(0, _jsCurPackageList["msg"][itCurPackageIcon]["iconUrl"].asString(), iconFileName)) {
				//add to icon list
				AddToIconsRepository(_jsCurPackageList["msg"][itCurPackageIcon]["id"].asString(), _jsCurPackageList["msg"][itCurPackageIcon], iconFileName);
			}
			else {
				packageIconsFailed.push_back(redoTask);
			}
			times--;
		}
	}
}
