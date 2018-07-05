// network.h
// Module to support UDP network for accepting commands and replying it 
// It spawn a background thread which keep listening UDP message.
// It provides supports replying to the received message 

#ifndef _NETWORK_H_
#define _NETWORK_H_

typedef enum{
    NoCommand,
    Stop,
    SetBpm,
    SetVolume,
    SetMode,
    Help,
    PlayHihat,
    PlayBase,
    PlaySnare,
    GetStatus,
    Invalid
} CommandType; 

// Begin the background thread which listens user commands
// return 0 for success
// return an error number for error
int Network_start (void);

// End the background thread which listens to user commands
void Network_end (void);

// Get the command which is waiting for being processed
// set num to be the second argument of UDP commands, SetBpm and Set Volume
// @commandType type possible returned value
//     SetBpm
//     SetVolume
//     Stop
void Network_checkCommand (CommandType *type, int *num);

// Reply To command. The program need to reply to every command (otherwise, it would be locked)
void Network_replyToCommand (char *message);

#endif
