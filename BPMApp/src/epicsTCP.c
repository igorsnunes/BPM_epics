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
	ret_val=1;
	epicsMutexUnlock(mutex);
	return ret_val;
}

//estabilish connection with ethernet device
int epics_TCP_connect(int instrument_id){
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

	if (check_status_ok(sock)==0)
		return 0;
	
	return 1;
}

//TODO:implement message with buffer
static int comm_talk(int sock,command_header ask,int size, command_header *answer){
	int ret_val = 0;
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
	size=var_read_command_ask(&ask);	
	ask.p[0]=(unsigned char)variable;
	//TODO:not the best way to treat this race condition
	//TODO:treat errors correctly
	return comm_talk(sock,ask,size,&answer);
	
}

//multiplex right operation
int epics_TCP_do(int sock, epicsUInt8 *buf, int instrument_id, int variable, enum operation op){
	switch(op){
		case OP_READ_BI:
			return get_bi(sock,buf,instrument_id,variable);
			break;
		case OP_READ_AI:
			break;
		case OP_WRITE_BO:
			break;
		case OP_WRITE_AO:
			break;
	}
	return 0;
}
