var socket = io.connect();

function setDrumMode(mode){
    sendCommand('setMode ' + mode);
}

function decreaseVolume(){
    var currentVolume = $('#currentVolume').text();
    var nextVolume = parseInt(currentVolume) -5;
    if ( 0 <= nextVolume && nextVolume <= 100){
        sendCommand('setVolume ' + nextVolume); 
        $('#currentVolume').text(nextVolume); 
    }
}

function increaseVolume(){
    var currentVolume = $('#currentVolume').text();
    var nextVolume = parseInt(currentVolume) + 5;
    if ( 0 <= nextVolume && nextVolume <= 100){
        $('#currentVolume').text(nextVolume); 
        sendCommand('setVolume ' + nextVolume); 
    }
}

function decreaseTempo(){
    var currentTempo = $('#currentTempo').text();
    var nextTempo = parseInt(currentTempo) - 5;
    if ( 40 <= nextTempo && nextTempo <= 300){
        $('#currentTempo').text(nextTempo);
        sendCommand('setBpm ' + nextTempo);
    }
}

function increaseTempo(){
    var currentTempo = $('#currentTempo').text();
    var nextTempo = parseInt(currentTempo) + 5;
    console.log(nextTempo);
    if ( 40 <= nextTempo && nextTempo <= 300){
        $('#currentTempo').text(nextTempo);
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
