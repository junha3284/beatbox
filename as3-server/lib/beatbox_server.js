"use strict";

/*
 * Refered : codes from cmpt 433 nodejs examples 
 * Created the module for udp communication with beatbox process
 */

var fs   = require('fs');
var socketio = require('socket.io');
var io;

var dgram = require('dgram');

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');
	
	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
};

function handleCommand(socket) {

    var PORT = 12345;
	var HOST = '127.0.0.1';

	// Pased string of comamnd to relay
	socket.on('sendCommand', function(data) {
		console.log('command: ' + data );
		
		// Info for connecting to the local process via UDP
		var buffer = new Buffer.from(data + '0');

		var client = dgram.createSocket('udp4');
        var timeout;
		client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
		    if (err) 
		    	throw err;
		    console.log('UDP message sent to ' + HOST +':'+ PORT + ' for command');
            timeout = setTimeout(function() {
                console.log('no respond from BBB');
                socket.emit('noRespond', 'no respond from BBB');
            }, 1000);
		});
		
		client.on('listening', function () {
		    var address = client.address();
		    console.log('UDP Client: listening on ' + address.address + ":" + address.port);
		});
		// Handle an incoming message over the UDP from the local application.
		client.on('message', function (message, remote) {
		    console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);
		    clearTimeout(timeout); 
		    var reply = message.toString('utf8')
		    socket.emit('commandReply', reply);
		    client.close();

		});
		client.on("UDP Client: close", function() {
		    console.log("closed");
		});
		client.on("UDP Client: error", function(err) {
		    console.log("error: ",err);
		});
	});

    socket.on('getCurrentStatus', function() {
		console.log('get current status');
		
		// Info for connecting to the local process via UDP
		
		var buffer = new Buffer.from('getStatus' + '0');

		var client = dgram.createSocket('udp4');
        var timeout;
		client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
		    if (err) 
		    	throw err;
		    console.log('UDP message sent to ' + HOST +':'+ PORT + ' for update');
            timeout = setTimeout(function() {
                console.log('no respond from BBB');
                socket.emit('noRespond', 'no respond from BBB');
            }, 1000);
		});
		
		client.on('listening', function () {
		    var address = client.address();
		    console.log('UDP Client: listening on ' + address.address + ":" + address.port);
		});
		// Handle an incoming message over the UDP from the local application.
		client.on('message', function (message, remote) {
		    console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);
		    clearTimeout(timeout); 
		    var reply = message.toString('utf8')
		    socket.emit('currentStatusReply', reply);
		    client.close();

		});
		client.on("UDP Client: close", function() {
		    console.log("closed");
		});
		client.on("UDP Client: error", function(err) {
		    console.log("error: ",err);
		});
	});

    socket.on('proc', function(fileName) {
		// NOTE: Very unsafe? Why?
		// Hint: think of ../
		var absPath = "/proc/" + fileName;
		
		fs.exists(absPath, function(exists) {
			if (exists) {
				// Can use 2nd param: 'utf8', 
				fs.readFile(absPath, function(err, fileData) {
					if (err) {
						emitSocketData(socket, fileName, 
								"ERROR: Unable to read file " + absPath);
					} else {
						emitSocketData(socket, fileName, 
								fileData.toString('utf8'));
					}
				});
			} else {
				emitSocketData(socket, fileName, 
						"ERROR: File " + absPath + " not found.");
			}
		});
	});
};

function emitSocketData(socket, fileName, contents) {
	var result = {
			fileName: fileName,
			contents: contents
	}
	socket.emit('fileContents', result);	
}
