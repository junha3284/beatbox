"use strict";
// Client-side interactions with the browser.

// Websocket connection to server
var socket = io.connect();
var timer;
// Make connection to server when web page is fully loaded.
$(document).ready(function() {
	console.log("Document loaded");

	// Setup a repeating function (every 1s)
	window.setInterval(function() {sendRequest('uptime')}, 1000);


	// Handle data coming back from the server
	socket.on('fileContents', function(result) {
        window.clearTimeout(timer);
		var fileName = result.fileName;
		var contents = result.contents;
		var domObj;
        var totalSecond = parseInt(contents.split(" ",1));
        var hours = Math.floor(totalSecond / (60*60));
        totalSecond %= (60*60);
        var mins = Math.floor(totalSecond / (60));
        var secs = totalSecond %60;
        $('#uptime').text(hours + ":" + mins + ":" + secs);
	});
});

function sendRequest(file) {
	console.log("Requesting '" + file + "'");
	socket.emit('proc', file);
    timer = window.setTimeout(function(){
        $('#uptime').text("no response from the server");
    }, 1500);
}

function replaceAll(str, find, replace) {
	return str.replace(new RegExp(find, 'g'), replace);
}
