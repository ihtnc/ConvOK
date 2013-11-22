var enable_logging = true;
var mode = 0;
var bt = 0;

Pebble.addEventListener("ready", function() {
	var temp_mode = localStorage.getItem("InvertMode");
	if (temp_mode) { 
		mode = temp_mode; 
		
		if(enable_logging) console.log("Pebble.ready: InvertMode=" + mode);
	}
	else {
		if(enable_logging) console.log("Pebble.ready: default InvertMode=" + mode);
	}

	var temp_bt = localStorage.getItem("BTNotification");
	if (temp_bt) { 
		bt = temp_bt; 
		
		if(enable_logging) console.log("Pebble.ready: BTNotification=" + bt);
	}
	else {
		if(enable_logging) console.log("Pebble.ready: default BTNotification=" + bt);
	}
});

Pebble.addEventListener("showConfiguration", function(e) {
	var json = 
		"[" +
			"{" +
				"'caption':'Screen color'," +
				"'key':'InvertMode'," +
				"'initialValue':'" + mode + "'," +
				"'type':'radiobutton'," +
				"'values':" +
				"[" + 
					"{" + 
						"'text':'Original'," +
						"'value':'0'" +
					"}," +
					"{" +
						"'text':'Invert in the morning'," +
						"'value':'1'" +
					"}," +
					"{" +
						"'text':'Invert'," +
						"'value':'2'" +
					"}" +
				"]" +
			"}," +
			"{" +
				"'caption':'BT Notification'," + 
				"'key':'BTNotification'," +
				"'type':'checkbox'," +
				"'initialValue':'" + bt + "'" +
			"}" +
		"]";
	var url = "http://ihtnc-pebble-config.azurewebsites.net/?";
	var title = "&title=OrangeKid+Configuration";
	var fields = "&fields=" + json;
	
	if(enable_logging) console.log("Pebble.showConfiguration: url=" + url + title + fields);
	Pebble.openURL(url + title + fields);
});

Pebble.addEventListener("webviewclosed", function(e) {
	if(!e.response && enable_logging) {
		console.log("Pebble.webviewclosed: no response received");
		return;
	}

	var configuration = JSON.parse(e.response);
	if(configuration["action"] = "cancel") {
		if(enable_logging) console.log("Pebble.webviewclosed: action=cancel");
		return;
	}
	
	if(enable_logging) console.log("Pebble.webviewclosed: action=save");
		
	Pebble.sendAppMessage(configuration);
	if(enable_logging) console.log("Pebble.sendAppMessage: done");
	
	mode = configuration["InvertMode"];
	localStorage.setItem("InvertMode", mode);
	if(enable_logging) console.log("Pebble.webviewclosed: mode=" + mode);
	    
	bt = configuration["BTNotification"];
	localStorage.setItem("BTNotification", bt);
	if(enable_logging) console.log("Pebble.webviewclosed: bt=" + bt);	
});
