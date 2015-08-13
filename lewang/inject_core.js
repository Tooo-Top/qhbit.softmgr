var remaining = 0;
var connected = false;

//! <!--  [ connect slots ] -->
function connectSlots() {
    if (!connected) {
        connected = true;
        DYBC.updateSoftCategory.connect(this, updateSoftCategory_callback);//更新分类数据
        DYBC.updateHotList.connect(this, updateHotList_callback);//更新热门分类(虚拟分类)数据
        DYBC.updateCategoryListForID.connect(this, updateCategoryListForID_callback);//按分类ID更新该分类下的软件清单
        DYBC.updatePackageInfoByID.connect(this, updatePackageInfoByID_callback);//更新指定ID的软件包信息

        DYBC.updateRegisteUser.connect(this, updateRegisteUser_callback);//处理注册用户的结果
        DYBC.updateLoginUser.connect(this, updateLoginUser_callback);//处理用户登录的结果
        DYBC.updateModifyUserInfo.connect(this, updateModifyUserInfo_callback);//处理用户资料修改的结果

        DYBC.updateRunningTasks.connect(this, updateRunningTasks_callback);//所有下载任务的状态清单, 包含正在进行的, 暂停的...
        DYBC.updateTaskInfo.connect(this, updateTaskInfo_callback);//某个软件包的状态更新
        DYBC.updateDownloadProgress.connect(this, updateDownloadProgress_callback);//某个软件包的下载进度
    }
}
//! <!--  [ connect slots ] -->


/*更新分类数据*/
updateSoftCategory_callback = function (data) {
    //console.log(data);
    var myTemplate = Handlebars.compile($("#xbSwmgrCategory-template").html());
    $('#xbSwmgrCategory').html(myTemplate(data));

/*
    var list = '';
    $(data).each(function (k, v) {
        console.log(k,v);
        list += '<a href="Encyclopedia.html?id=' + v.id + '" class="z' + v.id + '">' + v.name + '<span>(' + v.total + ')</span></a>';
        //DYBC.requestCategoryListByID(v.id);
    });
    $('#xbSwmgrCategory').html(list);
*/

};

/*更新热门分类(虚拟分类)数据*/
updateHotList_callback = function (packageList) {
    console.log(packageList);
};

/*按分类ID更新该分类下的软件清单*/
updateCategoryListForID_callback = function (categoryId, packageList) {
    //console.log(categoryId);
    //console.log(packageList);
    var myTemplate = Handlebars.compile($("#packageList-template").html());
    $('#packageList').html(myTemplate(packageList));
};

/*更新指定ID的软件包信息*/
updatePackageInfoByID_callback = function (packageInfo) {
    console.log(packageInfo);
};

/*所有下载任务的状态清单, 包含正在进行的, 暂停的...*/
updateRunningTasks_callback = function (taskList) {
    console.log(taskList);
};

/*某个软件包的状态更新*/
updateTaskInfo_callback = function (taskInfo) {
    console.log(taskInfo);
};

/*某个软件包的下载进度*/
updateDownloadProgress_callback = function (categoryId, packageId, percent) {
    console.log(categoryId);
    console.log(packageId);
    console.log(percent);
};

/*处理注册用户的结果*/
updateRegisteUser_callback = function (result) {
    console.log(result);
};

/*处理用户登录的结果*/
updateLoginUser_callback = function (result) {
    console.log(result);
};

/*处理用户资料修改的结果*/
updateModifyUserInfo_callback = function (result) {
    console.log(result);
};


(function ($){
    $.getUrlParam = function (name) {
        var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)");
        var r = window.location.search.substr(1).match(reg);
        if (r != null) return unescape(r[2]); return null;
    }
})(jQuery);

//init
$(function () {
    /*初始化信号槽*/
    connectSlots();

    /*请求更新分类*/
    //DYBC.requestSoftCategoryList();

    //用hotCategory填充首页软件清单
    //DYBC.requestHotList();
    //DYBC.requestCategoryListByID('1');


/*
    DYBC.requestSoftCategoryList();
    DYBC.requestHotList();
    DYBC.requestCategoryListByID(QString szCategoryID);
    DYBC.requestPackageInfoByID(QString szCategoryID,QString szPackageID);

    //about user
    DYBC.requestRegisteUser(QString username,QString password,QString email){}
    DYBC.requestLoginUser(QString username,QString password){}
    DYBC.requestModifyUserInfo(QVariant userinfo){}

    //soft package operation
    DYBC.requestStartInstallPackage(QString szCategoryID,QString szPackageID,bool autoInstall){} // software package download and install ( auto install ??)
    DYBC.requestBatStartInstallPackage(QVariantList lstPackage){}  // bat install
    DYBC.requestPausePackage(QString szCategoryID,QString szPackageID){} //pause someone package
    DYBC.requestResumePackage(QString szCategoryID,QString szPackageID){} //resume someone package
    DYBC.requestAllResumePackage(){} //resume all package
    DYBC.requestStopDownloadPackage(QString szCategoryID,QString szPackageID){}
*/

});

