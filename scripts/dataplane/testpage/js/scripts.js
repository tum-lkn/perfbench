var loadTimeStart = Date.now();
var redirect_interval = 2

$(window).load(function() {
	var loadTime = (Date.now() - loadTimeStart)/1000;
	$("#loadTime").html(loadTime+"s");
	
	// Redirect timer
	
	setTimeout(function(){window.location.replace("blank.html")},redirect_interval*1000);
});

$(function(){
	// Add all img files
	var count = 100;
	for (var i = 0; i < count; i++) {
			var tpl = "<img src='./img/ml/ml"+i+".png' />"
			$("#img_container").append(tpl);
	}
	
});


