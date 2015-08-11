var remaining = 0;
var connected = false;

//! <!--  [ connect slots ] -->
function connectSlots() {
    if (!connected) {
        connected = true;
        DYBC.putSoftCategory.connect(this, category_callback);
    }
}
//! <!--  [ connect slots ] -->


//requestSoftCategoryList callback
category_callback = function (data) {
    //console.log(data);
    var list = '';
    $(data).each(function(k,v){
        //console.log(k,v);
        list += '<a href="Encyclopedia.html?id='+ v.id+'" class="z'+ v.id+'">'+ v.name+'<span>('+ v.total+')</span></a>';
    });
    $('#xbSwmgrCategory').html(list);

};

$(function () {
    connectSlots();
    //$('#ct').click(function () {
		//处理分类
        DYBC.requestSoftCategoryList();
    //});

});

