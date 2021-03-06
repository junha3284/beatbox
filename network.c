#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "network.h"
#include "audioMixer.h"
#include "beatbox.h"

#define RECEIVING_MSG_MAX_LEN 64 
#define REPLYING_MSG_MAX_LEN 1024
#define MYPORT 12345
#define MAPPING_LENGTH 9 
#define BENCHMARK_PACKET_SIZE 1015 // for 'get array' command, while filling in Message for replying,
                                   // If Message for replying is larger than 1015 bytes, send it and create empty to fill left numbers in array. 

typedef struct {
    struct sockaddr_storage senderAddress;
    unsigned int addressSize;
    CommandType type;
    int requestedNum;
} Command;

const static struct {
    const char *str;
    CommandType type;
} mapping_string_command[] = {
    {"stop", Stop},
    {"setBpm", SetBpm},
    {"setVolume", SetVolume},
    {"setMode", SetMode},
    {"playHihat", PlayHihat},
    {"playBase", PlayBase},
    {"playSnare", PlaySnare}, 
    {"getStatus", GetStatus},
    {"help", Help}
};

static struct sockaddr_in my_addr; 
static int sockfd;

static Command currentCommand;

static pthread_t recvThread;
static pthread_mutex_t currentCommandLock = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t processingCommandCond = PTHREAD_COND_INITIALIZER;
static bool running;

// function declarations
// function for thread which will keep receiving user's command
static void* recvLoop (void*);
static CommandType stringToCommandMap (const char *string);
// Send the message to command sender
static int replyToSender (char *reply);

// Begin the background thread which listens user commands
// return 0 for success
// return an error number for error
int Network_start (void)
{
    currentCommand.type = NoCommand;
    currentCommand.requestedNum = 0;
    currentCommand.addressSize = sizeof (currentCommand.senderAddress);
    running = true;
    sockfd = socket (PF_INET, SOCK_DGRAM, 0);

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons (MYPORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset (my_addr.sin_zero, '\0', sizeof (my_addr.sin_zero));

    bind (sockfd, (struct sockaddr*) &my_addr, sizeof (my_addr));
    int threadCreateResult = pthread_create (&recvThread, NULL, recvLoop, NULL);
    return threadCreateResult;
}

// function for thread which will keep receiving user's command
static void* recvLoop (void* empty)
{
    printf ("start receviving command\n");
    while (running) {
        char Message[RECEIVING_MSG_MAX_LEN];
        int bytesRx;
        pthread_mutex_lock (&currentCommandLock);
        {
            bytesRx = recvfrom (sockfd,         // file descriptino for socket
                    Message,                    // Buffer for message input
                    RECEIVING_MSG_MAX_LEN,      // Size of Message Buffer
                    0,                          // Flags
                    (struct sockaddr*) &(currentCommand.senderAddress), // struct sockaddr* from
                    &(currentCommand.addressSize));// fromlen
        }
        pthread_mutex_unlock (&currentCommandLock);

        // to prevent out-of-bounds access
        Message[RECEIVING_MSG_MAX_LEN-1] = '\0';

        // to cut out 'enter' character from user input
        Message[bytesRx-1] = '\0';

        // get the first word from user's input
        char *token = strtok (Message, " ");

        if (bytesRx > 0){
            switch (stringToCommandMap (token)) {
                case Stop :
                {
                    running = false;
                    int i = replyToSender ("the program got stopped\n");
                    if (i <= 0){
                        running = true;
                        break;
                    }
                    pthread_mutex_lock (&currentCommandLock);
                    {
                        currentCommand.type = Stop;
                    }
                    pthread_mutex_unlock (&currentCommandLock);
                    break;
                }
                case GetStatus:
                {
                    char buf[32];
                    snprintf(buf, 32, "%d %d %d ", AudioMixer_getVolume(), Beatbox_getBpm(), Beatbox_getMode());
                    replyToSender(buf);
                    break;
                } 
                case SetBpm :
                {
                    // get the second word from user's input
                    token = strtok(NULL, " ");
                    if (token == NULL){
                        replyToSender ("there is no enough argument for Get command\n");
                        break;
                    }

                    char *endptr = NULL;
                    int requestedNum = strtol (token, &endptr, 10);

                    // check whether second argument includes non-numeric value or not
                    if (endptr != NULL){
                        // second argument include non-numeric value
                        if (*endptr != '\0'){
                            replyToSender ("Invalid argument for Get command (only numeric value for second argument)\n");
                            break;
                        }
                        // second argument does not include non-numeric value
                        pthread_mutex_lock (&currentCommandLock);
                        {
                            currentCommand.requestedNum = requestedNum; 
                            currentCommand.type = SetBpm;
                            pthread_cond_wait (&processingCommandCond, &currentCommandLock);
                        }
                        pthread_mutex_unlock (&currentCommandLock);
                        break;
                    }
                    printf("unknown error while verifying second argument for Get command\n");
                } 

                case SetVolume :
                {
                    // get the second word from user's input
                    token = strtok(NULL, " ");
                    if (token == NULL){
                        replyToSender ("there is no enough argument for Get command\n");
                        break;
                    }

                    char *endptr = NULL;
                    int requestedNum = strtol (token, &endptr, 10);

                    // check whether second argument includes non-numeric value or not
                    if (endptr != NULL){
                        // second argument include non-numeric value
                        if (*endptr != '\0'){
                            replyToSender ("Invalid argument for Get command (only numeric value for second argument)\n");
                            break;
                        }
                        // second argument does not include non-numeric value
                        pthread_mutex_lock (&currentCommandLock);
                        {
                            currentCommand.requestedNum = requestedNum; 
                            currentCommand.type = SetVolume;
                            pthread_cond_wait (&processingCommandCond, &currentCommandLock);
                        }
                        pthread_mutex_unlock (&currentCommandLock);
                        break;
                    }
                    printf("unknown error while verifying second argument for Get command\n");
                }

                case SetMode :
                {
                    // get the second word from user's input
                    token = strtok(NULL, " ");
                    if (token == NULL){
                        replyToSender ("there is no enough argument for Get command\n");
                        break;
                    }

                    char *endptr = NULL;
                    int requestedNum = strtol (token, &endptr, 10);

                    // check whether second argument includes non-numeric value or not
                    if (endptr != NULL){
                        // second argument include non-numeric value
                        if (*endptr != '\0'){
                            replyToSender ("Invalid argument for Get command (only numeric value for second argument)\n");
                            break;
                        }
                        // second argument does not include non-numeric value
                        pthread_mutex_lock (&currentCommandLock);
                        {
                            currentCommand.requestedNum = requestedNum; 
                            currentCommand.type = SetMode;
                            pthread_cond_wait (&processingCommandCond, &currentCommandLock);
                        }
                        pthread_mutex_unlock (&currentCommandLock);
                        break;
                    }
                    printf("unknown error while verifying second argument for Get command\n");
                }
                case PlayHihat :
                {
                    pthread_mutex_lock (&currentCommandLock);
                    {
                        currentCommand.type = PlayHihat;
                        pthread_cond_wait (&processingCommandCond, &currentCommandLock);
                    }
                    pthread_mutex_unlock (&currentCommandLock);
                    break;
                }
                case PlayBase:
                {
                    pthread_mutex_lock (&currentCommandLock);
                    {
                        currentCommand.type = PlayBase;
                        pthread_cond_wait (&processingCommandCond, &currentCommandLock);
                    }
                    pthread_mutex_unlock (&currentCommandLock);
                    break;
                }
                case PlaySnare :
                {
                    pthread_mutex_lock (&currentCommandLock);
                    {
                        currentCommand.type = PlaySnare;
                        pthread_cond_wait (&processingCommandCond, &currentCommandLock);
                    }
                    pthread_mutex_unlock (&currentCommandLock);
                    break;
                }
                case Help:
                {
                    replyToSender ("Accepted command examples:\n"
                            "\tsetBpm # -- set Bpm to # (40-300).\n"
                            "\tsetVolume # -- set volume to # (0-100).\n"
                            "\tplayBase -- play base once.\n"
                            "\tplaySnare -- play snare once.\n"
                            "\tplayHihat -- play hihat once.\n");
                    break;
                }
                case Invalid:
                {
                    replyToSender ("Invalid command is received!\n");
                    break;
                }
                default :
                    printf ("unknown error while mapping command!\n");
            }
        }
        else
            printf ("error for recvfrom\n");
    }
    printf ("stop receiving command\n");
    return NULL;
}

static CommandType stringToCommandMap (const char *string)
{
    if (string == NULL)
        return Invalid;
    for (int i = 0; i < MAPPING_LENGTH; i++){
        if (strcmp (mapping_string_command[i].str, string) == 0)
            return mapping_string_command[i].type;
    }
    return Invalid;
}

// End the background thread which listens to user commands
void Network_end (void)
{
    pthread_join(recvThread, NULL);
    close(sockfd);
}

// Get the command which is waiting for being processed
// set num to be # for command 'get #' if current command is get #
// @commandType type possible returned value
//     NoCommand
//     SetBpm
//     SetVolume
//     Stop
void Network_checkCommand (CommandType *type, int *num)
{
    pthread_mutex_lock (&currentCommandLock);
    {
        // If the current command is 'get #', send the caller the # through num variable
        if (currentCommand.type == SetBpm
                || currentCommand.type == SetVolume
                || currentCommand.type == SetMode)
            *num = currentCommand.requestedNum;
        *type = currentCommand.type;
    }
    pthread_mutex_unlock (&currentCommandLock);
}

void Network_replyToCommand (char *message)
{
    pthread_mutex_lock (&currentCommandLock);
    {
        replyToSender(message);
        currentCommand.type = NoCommand;
        pthread_cond_signal (&processingCommandCond);
    }
    pthread_mutex_unlock (&currentCommandLock);
}

// Send the message to command sender
static int replyToSender (char *reply)
{
    if (reply == NULL || reply[0] == '\0')
        return 0;

    int bytesRx = sendto(sockfd,
            reply,                          // message the program want to send
            strlen(reply)+1,                  // size of message
            0,                              // Flags
            (struct sockaddr*) &(currentCommand.senderAddress),   // struct sockaddr* from
            currentCommand.addressSize);    // fromlen

    if (bytesRx <= 0) {
        printf ("error happend while replying\n");
    }
    return bytesRx;
}
