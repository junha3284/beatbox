var socket = io.connect();



$(document).ready(function(){

	window.setInterval(function() {socket.emit('getCurrentStatus')}, 500);
    socket.on('noRespond', function(data){
        $('#connectionStatus').text(data); 
    });
    socket.on('currentStatusReply', function(reply){
        var Status = reply.split(' ');
        $('#currentVolume').text(Status[0]); 
        $('#currentTempo').text(Status[1]);
        $('#connectionStatus').text('connected to BBB');
        var mode = Status[2];
        if (mode == 0)
            $('#currentMode').text('Rock #1');
        else if (mode == 1)
            $('#currentMode').text('Rock #2');
        else
            $('#currentMode').text('None');
    });
});


function setDrumMode(mode){
    sendCommand('setMode ' + mode);
}

function decreaseVolume(){
    var currentVolume = $('#currentVolume').text();
    var nextVolume = parseInt(currentVolume) -5;
    if ( 0 <= nextVolume && nextVolume <= 100){
        sendCommand('setVolume ' + nextVolume); 
    }
}

function increaseVolume(){
    var currentVolume = $('#currentVolume').text();
    var nextVolume = parseInt(currentVolume) + 5;
    if ( 0 <= nextVolume && nextVolume <= 100){
        sendCommand('setVolume ' + nextVolume); 
    }
}

function decreaseTempo(){
    var currentTempo = $('#currentTempo').text();
    var nextTempo = parseInt(currentTempo) - 5;
    if ( 40 <= nextTempo && nextTempo <= 300){
        sendCommand('setBpm ' + nextTempo);
    }
}

function increaseTempo(){
    var currentTempo = $('#currentTempo').text();
    var nextTempo = parseInt(currentTempo) + 5;
    if ( 40 <= nextTempo && nextTempo <= 300){
        sendCommand('setBpm ' + nextTempo);
    }
}

function playHihat(){
    sendCommand("playHihat");
}

function playBase(){
    sendCommand("playBase");
}

function playSnare(){
    sendCommand("playSnare");
}

function sendCommand(data){
    socket.emit('sendCommand', data);
};
