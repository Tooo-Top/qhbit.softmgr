$(document).ready(function(){
	$("#firstpane .menu_body:eq(0)").show();
	$("#firstpane h3.menu_head").click(function(){
		$(this).addClass("current").next("div.menu_body").slideToggle(300).siblings("div.menu_body").slideUp("slow");
		$(this).siblings().removeClass("current");
	});
	
	$("#secondpane .menu_body:eq(0)").show();
	$("#secondpane h3.menu_head").mouseover(function(){
		$(this).addClass("current").next("div.menu_body").slideDown(500).siblings("div.menu_body").slideUp("slow");
		$(this).siblings().removeClass("current");
	});
	
});

$(function(){
	var Bool=false;
	var Scro=$("#scro");
	var Scrp=$("#pks");
	var Scrobd=$("#softid");
	var Scroul=$("#ulid");
	var Scrp_Height =Scrp.outerHeight()/2;
	var Num2=Scro.outerHeight()-Scrp.outerHeight();
	var offsetX=0;
	var offsetY=0;
	Scrp.mousedown(function(e){  
		Bool=true;
	});
	$(document).mouseup(function(){
		Bool=false;
	});
	$(document).mousemove(function(e){
		if(Bool){
			var Num1= e.clientY - Scro.position().top;
			var y=Num1 - Scrp_Height;
			if(y<=1){
				Scrll(0);
				Scrp.css("topid",1);
			}else if(y>=Num2){
				Scrp.css("top",Num2);
				Scrll(Num2);
			}else{
				Scrll(y);
			}
		}
	});
	function Scrll(y){
		Scrp.css("top",y);
		Scroul.css("margin-top",-(y/(Scro.outerHeight()-Scrp.outerHeight()))*(Scroul.outerHeight()-Scrobd.height()));
	}
	if(document.getElementById("scroll_set").addEventListener) 
	document.getElementById("scroll_set").addEventListener('DOMMouseScroll',wheel,true);
	document.getElementById("scroll_set").onmousewheel=wheel;
	var Distance=Num2*0.1;
	function wheel(e){
		var evt = e || window.event;
		var wheelDelta = evt.wheelDelta || evt.detail;
		if(wheelDelta == -120 || wheelDelta == 3){
			var Distances=Scrp.position().top+Distance;
			if(Distances>=Num2){
				Scrp.css("top",Num2);
				Scrll(Num2);
			}else{
				Scrll(Distances);
			}
			return false;
		}else if (wheelDelta == 120 || wheelDelta == -3){
			var Distances=Scrp.position().top-Distance;
			if(Distances<=1){
				Scrll(0);
				Scrp.css("topid",1);
			}else{
				Scrll(Distances);
			}
			return false;
		}   
	}
});






$(function(){
	var Bool=false;
	var Scro=$("#scroll");
	var Scrp=$("#p");
	var Scrobd=$("#bd");
	var Scroul=$("#ul");
	var Scrp_Height =Scrp.outerHeight()/2;
	var Num2=Scro.outerHeight()-Scrp.outerHeight();
	var offsetX=0;
	var offsetY=0;
	Scrp.mousedown(function(e){  
		Bool=true;
	});
	$(document).mouseup(function(){
		Bool=false;
	});
	$(document).mousemove(function(e){
		if(Bool){
			var Num1= e.clientY - Scro.position().top;
			var y=Num1 - Scrp_Height;
			if(y<=1){
				Scrll(0);
				Scrp.css("top",1);
			}else if(y>=Num2){
				Scrp.css("top",Num2);
				Scrll(Num2);
			}else{
				Scrll(y);
			}
		}
	});
	function Scrll(y){
		Scrp.css("top",y);
		Scroul.css("margin-top",-(y/(Scro.outerHeight()-Scrp.outerHeight()))*(Scroul.outerHeight()-Scrobd.height()));
	}
	if(document.getElementById("scroll_bd").addEventListener) 
	document.getElementById("scroll_bd").addEventListener('DOMMouseScroll',wheel,true);
	document.getElementById("scroll_bd").onmousewheel=wheel;
	var Distance=Num2*0.1;
	function wheel(e){
		var evt = e || window.event;
		var wheelDelta = evt.wheelDelta || evt.detail;
		if(wheelDelta == -120 || wheelDelta == 3){
			var Distances=Scrp.position().top+Distance;
			if(Distances>=Num2){
				Scrp.css("top",Num2);
				Scrll(Num2);
			}else{
				Scrll(Distances);
			}
			return false;
		}else if (wheelDelta == 120 || wheelDelta == -3){
			var Distances=Scrp.position().top-Distance;
			if(Distances<=1){
				Scrll(0);
				Scrp.css("top",1);
			}else{
				Scrll(Distances);
			}
			return false;
		}   
	}
});
