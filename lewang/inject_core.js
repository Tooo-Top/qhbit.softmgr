var remaining = 0;
var connected = false;

//! <!--  [ connect slots ] -->
function connectSlots()
{
	if ( !connected ) {
		connected = true;
		DYBC.putSoftCategory.connect(this, category_callback);
	}
}
//! <!--  [ connect slots ] -->


//requestSoftCategoryList callback
category_callback=function(data){
	alert('category_callback');
	console.log(data);
};

$(function(){
	connectSlots();
	$('#ct').click(function(){
		DYBC.requestSoftCategoryList();
	});
	
});

