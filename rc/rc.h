// rc.h

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>

static const int RC_PORT = 11989;
static const int RC_MAX_BUFFER_SIZE	= 512;

enum {
	RC_OPCODE_SAVE
} rc_opcode;

typedef struct _rc {
	int isRunning;
	
	struct sockaddr_in address;
	struct sockaddr_in remote;
	int socket;
	pthread_t acceptor, receiver;
	
	int client;
	int isClientConnected;
} RemoteControl;

int rc_start(RemoteControl* rc);
void rc_stop(RemoteControl* rc);

void *rc_acceptor(void* args);
void *rc_receiver(void* args);

void rc_save(int slot);
