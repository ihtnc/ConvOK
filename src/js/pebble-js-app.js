var enable_logging = false;
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
	var fields = "&fields=" + encodeURIComponent(json);
	
	if(enable_logging) console.log("Pebble.showConfiguration: url=" + url + title + fields);
	Pebble.openURL(url + title + fields);
});

Pebble.addEventListener("webviewclosed", function(e) {
	if(!e.response && enable_logging) {
		console.log("Pebble.webviewclosed: no response received");
		return;
	}
	else {
		console.log("Pebble.webviewclosed: response=" + e.response);
	}

	var configuration = JSON.parse(e.response);
	if(configuration["action"] == "cancel") {
		if(enable_logging) console.log("Pebble.webviewclosed: action=cancel");
		return;
	}
	
	if(enable_logging) console.log("Pebble.webviewclosed: action=save");
	
	mode = configuration["InvertMode"];
	localStorage.setItem("InvertMode", mode);
	if(enable_logging) console.log("Pebble.webviewclosed: mode=" + mode);
	
	if(configuration["BTNotification"] == null) bt = 0;
	else bt = configuration["BTNotification"];

	localStorage.setItem("BTNotification", bt);
	if(enable_logging) console.log("Pebble.webviewclosed: bt=" + bt);	

	//since thinCFG returne everything as string, convert the values we retrieved to int before sending to the pebble watchface
	configuration["InvertMode"] = parseInt(mode);
	configuration["BTNotification"] = parseInt(bt);
	Pebble.sendAppMessage(configuration);
	
	if(enable_logging) console.log("Pebble.sendAppMessage: done");
});
