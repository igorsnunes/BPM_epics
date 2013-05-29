#include "epicsTCP.h"

static epicsMutexId mutex;

static int check_status_ok(int sock){
	command_header cmmd,cmmd_answer;
	int size_cmmd;
	int ret_val;
	ok_command(&cmmd_answer);
	size_cmmd=status_command_ask(&cmmd);

	epicsMutexLock(mutex);
	
	if (send(sock, &cmmd, size_cmmd, 0)>0){
		perror("check_status_ok: failed send");
		ret_val=0;
	}
	if (recv(sock, &cmmd, size_cmmd, 0)<0){
		perror("check_status_ok: failed rcv");
		ret_val=0;
	}

	if (cmmd_answer.command != cmmd.command){
		perror("check_status_ok: wrong answer");
		ret_val=0;
	}
	ret_val=1
	epicsMutexUnlock(mutex);
	return ret_val;
}

//estabilish connection with ethernet device
int epics_TCP_connect(int instrument_id){
	int rc;
	struct sockaddr_in servaddr;
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(LPCPORT);
	//TODO:implement getting ip from system call using instrument id
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0){
		perror("epics_TCP_connect:connection failed");
		return 0;
	}

	if (check_status_ok(int sock)==0)
		return 0;
	
	return 1;
}

//multiplex right operation
int epics_TCP_do(int sock, epicsUInt8 *buf, int instrument_id, int variable, enum operation){
	switch(operation){
		case OP_READ_BI:
			return get_bi(sock,buf,instrument,variable);
		case OP_READ_AI:
		case OP_READ_BO:
		case OP_WRITE_AO:

	}
}
static int comm_talk(int sock,command_header ask,int size, command_header *answer){
	epicsMutexLock(mutex);
	if(send(sock, &ask, size, 0)<=0){
		perror("epics_TCP_get:message not sent");
		ret_val=0;
	}
	if(recv(sock,answer,sizeof(answer),0)){
		perror("epics_TCP_get:message not recv");
		ret_val=0;
	}
	ret_val=1;
	epicsMutexUnlock(mutex);
	return ret_val;
}

//get message from server
static int get_bi(int sock, epicsUInt8 *buf, int instrument_id, int variable){
	command_header ask,answer;
	int size;
	int ret_val;
	size=var_read_command_ask(&ask);	
	ask.p[0]=(unsigned char)variable;
	//TODO:not the best way to treat this race condition
	//TODO:treat errors correctly
	return comm_talk(sock,buf,size,&answer);
	
}
