#include "epicsTCP.h"

static pNode p;

int init_pNode(void);

static const iocshFuncDef init_pNodeDef = {"init_pNode",0,NULL};

static void init_pNodeFunc (const iocshArgBuf *args){
	init_pNode();
}

static void connRegister(void){
	iocshRegister(&init_pNodeDef,init_pNodeFunc);
}

epicsExportRegistrar(connRegister);


int init_pNode(void){
	p.first = NULL;
	printf("init");
	p.nnode = 0;
	return 1;
}

epicsMutexId mutListMng;

static int check_status_ok(int sock,epicsMutexId mutex){
	command_header cmmd,cmmd_answer;
	int size_cmmd;
	int ret_val = 1;
	status_command_answer(&cmmd_answer);
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

	printf("answer: %d %d\n",cmmd.command,cmmd.size);
	if (cmmd_answer.command != cmmd.command){
		perror("check_status_ok: wrong answer");
		ret_val=0;
	}
	epicsMutexUnlock(mutex);
	return ret_val;
}

static Node* create_Node(int device_id, int sock){
	Node *aux,*last;
	epicsMutexLock(mutListMng);
	p.nnode++;
	if (p.first == NULL){
		last = p.first = (Node*)malloc(sizeof(Node));
		
	}
	else{
		aux = p.first;
		while(aux->next != NULL)
			aux = aux->next;
		last = aux->next = (Node*)malloc(sizeof(Node));
	}
	last->sock = sock;
	last->device_id = device_id;
	last->mutex = epicsMutexCreate();
	last->next = NULL;
	epicsMutexUnlock(mutListMng);
	return last;
}

static int get_sock(int instrument_id){
	Node *aux = p.first;
	while(aux != NULL){
		if(instrument_id == aux->device_id)
			return aux->sock;
		aux = aux->next;
	}
	return -1;
}

static epicsMutexId get_mutex(int instrument_id){
	Node *aux = p.first;

	while(aux != NULL){
		printf("%d == %d\n", instrument_id,aux->device_id);
		if(instrument_id == aux->device_id)
			break;
		aux = aux->next;
	}
	return aux->mutex;
}

//estabilish connection with ethernet device
int epics_TCP_connect(int instrument_id, int *sock, int mut){
	struct sockaddr_in servaddr;
	int auxsock;
	Node *last = NULL;
	epicsMutexId mutr;
	mutListMng = epicsMutexCreate();
	auxsock = get_sock(instrument_id);
	if (auxsock != -1){
		printf("sock got\n");
		sock[0] = auxsock;
		mutr = get_mutex(instrument_id);
		if (check_status_ok(sock[0],mutr)==0){
			printf("Device status: not OK!\n");
			return 0;
		}
	}
	else{
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(LPCPORT);
		//TODO:implement getting ip from system call using instrument id
		servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		//servaddr.sin_addr.s_addr = inet_addr("10.0.17.201");
	
	
		if ((sock[0] = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			perror("epics_TCP_connect:socket failed");
			return 0;
		}
	
		if (connect(sock[0], (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0){
			perror("epics_TCP_connect:connection failed\n");
			return 0;
		}
		last = create_Node(instrument_id,sock[0]);
		if (check_status_ok(sock[0],last->mutex)==0){
			printf("Device status: not OK!\n");
			return 0;
		}
	}
	
	return 1;
}

static int comm_talk(int sock,command_header ask,int size, command_header *answer, int instrument_id){
	int ret_val = 1;
	epicsMutexId mutex;
	mutex = get_mutex(instrument_id);
	epicsMutexLock(mutex);
	if(send(sock, &ask, size, 0)<=0){
		perror("epics_TCP_get:message not sent");
		ret_val=0;
	}
	if(recv(sock,answer,sizeof(answer),0)<=0){
		perror("epics_TCP_get:message not recv");
		ret_val=0;
	}
	epicsMutexUnlock(mutex);
	printf("message received:%d %d %d\n",answer->command,answer->size,answer->p[0]);
	return ret_val;
}

//get message from server
static int get_bi_ai(int sock, epicsUInt8 **buf, int instrument_id, int variable){
	command_header ask,answer;
	int size,ret_val;
	size = var_read_command_ask(&ask);	
	ask.p[0] = (unsigned char)variable;
	ret_val = comm_talk(sock,ask,size,&answer,instrument_id);
	
	*buf = (epicsUInt8*)malloc(sizeof(epicsUInt8)*answer.size);
	memcpy(*buf,answer.p,answer.size);
	
	return ret_val;
}

static int get_bo(int sock, epicsUInt8 **buf, int instrument_id, int variables){
	command_header ask,answer;
	int  size, ret_val;
	variables = 1;
	size = var_write_command_ask(&ask,1);
	ask.p[0] = (unsigned char)variables;
	ask.p[1] = (unsigned char)*buf[0];
	
	ret_val = comm_talk(sock,ask,size,&answer,instrument_id);
	return ret_val;
}

static int get_operation(int sock, epicsUInt8 **buf, int instrument_id, int variable,enum operation op){
	
	switch(op){
		case OP_READ_BI:
			return get_bi_ai(sock,buf,instrument_id,variable);
			break;
		case OP_READ_AI:
			return get_bi_ai(sock,buf,instrument_id,variable);
			break;
		case OP_WRITE_BO:
			return get_bo(sock,buf,instrument_id,variable);	
			break;
		case OP_WRITE_AO:
			break;
	}
	return 0;
}

//multiplex right operation
int epics_TCP_do(int sock, epicsUInt8 **buf, int instrument_id, int variable, enum operation op){

	return get_operation(sock,buf,instrument_id,variable,op);
}
