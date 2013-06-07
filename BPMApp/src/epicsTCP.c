#include "epicsTCP.h"

static epicsMutexId mutex;

static int check_status_ok(int sock){
	command_header cmmd,cmmd_answer;
	int size_cmmd;
	int ret_val;
	ok_command(&cmmd_answer);
	size_cmmd=status_command_ask(&cmmd);

	epicsMutexLock(mutex);
	
	if (send(sock, &cmmd, size_cmmd, 0)<0){
		perror("check_status_ok: failed send");
		ret_val=0;
	}
	if (recv(sock, &cmmd, size_cmmd, 0)<0){
		perror("check_status_ok: failed rcv");
		ret_val=0;
	}

	printf("answer: %d %d",cmmd.command,cmmd.size);
	if (cmmd_answer.command != cmmd.command){
		perror("check_status_ok: wrong answer");
		ret_val=0;
	}
	ret_val=1;
	epicsMutexUnlock(mutex);
	return ret_val;
}

//estabilish connection with ethernet device
int epics_TCP_connect(int instrument_id, int *sock, int mut){
	struct sockaddr_in servaddr;

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(LPCPORT);
	//TODO:implement getting ip from system call using instrument id
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//servaddr.sin_addr.s_addr = inet_addr("10.0.17.201");
	
	if (mut)
		mutex = epicsMutexCreate();
	
	epicsMutexLock(mutex);
	if ((sock[0] = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("epics_TCP_connect:socket failed");
		return 0;
	}
	
	if (connect(sock[0], (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0){
		perror("epics_TCP_connect:connection failed\n");
		return 0;
	}
	
	if (check_status_ok(sock[0])==0){
		printf("Device status: not OK!\n");
		return 0;
	}
	
	epicsMutexUnlock(mutex);
	return 1;
}

//TODO:implement message with buffer
static int comm_talk(int sock,command_header ask,int size, command_header *answer){
	int ret_val = 1;
	if(send(sock, &ask, size, 0)<=0){
		perror("epics_TCP_get:message not sent");
		ret_val=0;
	}
	if(recv(sock,answer,sizeof(answer),0)<=0){
		perror("epics_TCP_get:message not recv");
		ret_val=0;
	}
	ret_val=1;
	printf("message received:%d %d %d\n",answer->command,answer->size,answer->p[0]);
	return ret_val;
}

//get message from server
static int get_bi(int sock, epicsUInt8 **buf, int instrument_id, int variable){
	command_header ask,answer;
	int size,ret_val;
	size=var_read_command_ask(&ask);	
	ask.p[0]=(unsigned char)variable;
	//TODO:not the best way to treat this race condition
	//TODO:treat errors correctly

	ret_val = comm_talk(sock,ask,size,&answer);
	*buf = (epicsUInt8*)malloc(sizeof(epicsUInt8)*answer.size);
	memcpy(*buf,answer.p,answer.size);
	return ret_val;
	
}
static int get_operation(int sock, epicsUInt8 **buf, int instrument_id, int variable,enum operation op){
	
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
static int do_by_device(int sock, epicsUInt8 **buf, int instrument_id, int variable,enum operation op){
	int ret_val = 0;
	switch(instrument_id){
		case 0:
			epicsMutexLock(mutex);
			ret_val = get_operation(sock,buf,instrument_id,variable,op);
			epicsMutexUnlock(mutex);
			break;
		case 1:
			epicsMutexLock(mutex);
			ret_val = get_operation(sock,buf,instrument_id,variable,op);
			epicsMutexUnlock(mutex);
			break;
		case 2:
			epicsMutexLock(mutex);
			ret_val = get_operation(sock,buf,instrument_id,variable,op);
			epicsMutexUnlock(mutex);
			break;

		case 3:
			epicsMutexLock(mutex);
			ret_val = get_operation(sock,buf,instrument_id,variable,op);
			epicsMutexUnlock(mutex);
			break;
		case 4:
			epicsMutexLock(mutex);
			ret_val = get_operation(sock,buf,instrument_id,variable,op);
			epicsMutexUnlock(mutex);
			break;
		case 5:
			epicsMutexLock(mutex);
			ret_val = get_operation(sock,buf,instrument_id,variable,op);
			epicsMutexUnlock(mutex);
			break;
		case 6:
			epicsMutexLock(mutex);
			ret_val = get_operation(sock,buf,instrument_id,variable,op);
			epicsMutexUnlock(mutex);
			break;
		case 7:
			epicsMutexLock(mutex);
			ret_val = get_operation(sock,buf,instrument_id,variable,op);
			epicsMutexUnlock(mutex);
			break;
	}
	return ret_val;
}
//multiplex right operation
int epics_TCP_do(int sock, epicsUInt8 **buf, int instrument_id, int variable, enum operation op){

	return do_by_device(sock,buf,instrument_id,variable,op);
}
