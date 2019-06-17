#include "rc.h"

#include "../frontend/main.h"

int rc_start(RemoteControl* rc)	{
	int result;
	
	rc->isRunning = 0;

    if ((rc->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }

    rc->address.sin_family = AF_INET;
    rc->address.sin_addr.s_addr = INADDR_ANY;
    rc->address.sin_port = htons(RC_PORT);

    int data = 1;
    if (setsockopt(rc->socket, SOL_SOCKET, SO_REUSEADDR, &data, sizeof(int)) < 0) {
        close(rc->socket);
        return -1;
    }

    result = bind(rc->socket, (struct sockaddr *)&rc->address , sizeof(rc->address));
    if (result != 0 ) {
        close(rc->socket);
        return -1;
    }

    result = listen(rc->socket, SOMAXCONN);
    if (result != 0) {
        close(rc->socket);
        return -1;
    }

    rc->isRunning = 1;
    
    pthread_create(&rc->acceptor, NULL, rc_acceptor, rc);

	return (rc->isRunning);
}

void rc_stop(RemoteControl* rc)	{
	int client;
	struct sockaddr_in address;
	
	if(rc->isRunning == 1) {
	
		printf("rc_stop\n");
		
		close(rc->socket);
		rc->isRunning = 0;	
		
		if((client = socket(AF_INET,SOCK_STREAM,0)!=0))  {
			address.sin_addr.s_addr = inet_addr("127.0.0.1");
			address.sin_family = AF_INET;
			address.sin_port = htons(RC_PORT);

			connect(client, (struct sockaddr *)&address , sizeof(address));
		}
	}
}

void *rc_acceptor(void* args)	{
	RemoteControl* rc = (RemoteControl*)args;
	socklen_t c;
	int socket;
	
	printf("rc_acceptor: started\n");
	
	while (rc->isRunning == 1) {
		socket = accept(rc->socket, (struct sockaddr *)&rc->remote, &c);
		if (rc->isRunning == 1) {
			if (rc->isClientConnected == 0) {
				rc->client = socket;
				rc->isClientConnected = 1;
				
				pthread_create(&rc->receiver, NULL, rc_receiver, rc);
			}
			else {
				printf("rc_acceptor: rc is busy\n");
			}
		}
	}
	
	printf("rc_acceptor: finished\n");
	
	pthread_exit(NULL);
	return NULL;
}

void *rc_receiver(void* args) {
	RemoteControl* rc = (RemoteControl*)args;
	char buffer[RC_MAX_BUFFER_SIZE];
	int bytes = 0;
	int quitReceiver = 0;
		
	while (quitReceiver == 0) {
		bytes = recv(rc->client, buffer, RC_MAX_BUFFER_SIZE, 0);
		if (bytes > 0) {
			if (buffer[0] == RC_OPCODE_SAVE && bytes == 2) {
				char slot = buffer[1];
				rc_save((int)slot);
			}
			else {
				printf("rc_receiver: unknown opcode %d\n", (int)buffer[0]);
			}
		}
		else {
		
			quitReceiver = 1;

			if (bytes == 0) {
				printf("rc_receiver: connection closed\n");
			}
			else {
				printf("rc_receiver: connection error %d\n", errno);				
			}
		}
	}
	
	rc->isClientConnected = 0;
	rc->client = 0;
	
	pthread_exit(NULL);
	return NULL;
}

void rc_save(int slot) {
	printf("rc_save: %d\n", slot);
	emu_save_state(0);
}



