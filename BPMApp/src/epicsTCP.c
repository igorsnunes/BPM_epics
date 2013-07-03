#include "epicsTCP.h"

static char ipTable[50][18];
static pNode p;
static epicsMutexId mutListMng;
int init_pNode(void);
int get_ips(char *ip, char *id);

static const iocshArg get_ipsArg0 = { "ip", iocshArgString };
static const iocshArg get_ipsArg1 = { "id", iocshArgString };

static const iocshArg * const get_ipsArgs[] = { &get_ipsArg0,&get_ipsArg1 };

static const iocshFuncDef init_pNodeDef = {"init_pNode",0,NULL};
static const iocshFuncDef get_ipsDef = {"get_ips",2,get_ipsArgs};

static void get_ipsFunc(const iocshArgBuf *args){
	get_ips(args[0].sval,args[1].sval);
}

static void init_pNodeFunc (const iocshArgBuf *args){
	init_pNode();
}

static void connRegister(void){
	iocshRegister(&init_pNodeDef,init_pNodeFunc);
	iocshRegister(&get_ipsDef,get_ipsFunc);
}

epicsExportRegistrar(connRegister);


int init_pNode(void){
	mutListMng = epicsMutexCreate();
	p.first = NULL;
	return 1;
}

int get_ips(char *id,char *ip){
	int idint = atoi(id);
	sprintf(ipTable[idint],"%s",ip);
	return 1;
}

static Node* create_Node(int device_id, int sock){
	Node *aux,*last;
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
	return last;
}
static void close_connection(int instrument_id){
	int socket = -1;
	if(p.first == NULL)
		return;
	p.nnode--;

	Node *aux = p.first,*prev = aux;
	if (p.first->next == NULL && p.first->device_id == instrument_id){
		socket = p.first->sock;
		close(socket);
		free(p.first);
		p.first = NULL;	
	}
	else{
		while(aux != NULL){
			if (aux->device_id == instrument_id){
				socket = aux->sock;
				close(socket);
				prev->next = aux->next;
				free(aux);
				break;
			}
			prev = aux;
			aux = aux->next;
		}
	}
	
	return;
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

static epicsMutexId get_mutex(int instrument_id,int *found){
	Node *aux = p.first;
	if(aux == NULL){
		found[0] = 0;
		return NULL;
	}

	while(aux != NULL){
		if(instrument_id == aux->device_id){
			found[0] = 1;
			return aux->mutex;
			break;
		}
		aux = aux->next;
	}
	found[0] = 0;
	return NULL;
}

static int list_operation(enum ListOperation op, epicsMutexId *mut, int *sock, Node **node, int instrument_id){
	epicsMutexLock(mutListMng);
	int ret_val = 0;
	switch (op){
		case GET_MUTEX:
			mut[0] = get_mutex(instrument_id,&ret_val);
			break;
		case GET_SOCK:
			sock[0] = get_sock(instrument_id);
			break;
		case CREATE_NODE:
			*node = create_Node(instrument_id, sock[0]);
			break;
		case REMOVE_NODE:
			close_connection(instrument_id);
			break;
	}
	epicsMutexUnlock(mutListMng);
	return ret_val;
}

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

	if (cmmd_answer.command != cmmd.command){
		perror("check_status_ok: wrong answer");
		ret_val=0;
	}
	epicsMutexUnlock(mutex);
	return ret_val;
}

//estabilish connection with ethernet device
int epics_TCP_connect(int instrument_id){
	struct sockaddr_in servaddr;
	struct timeval timeout;
	int sock;
	Node *last = NULL;
	epicsMutexId mutr;
	list_operation(GET_SOCK,NULL,&sock,NULL,instrument_id);
	printf("%d %d \n",sock,instrument_id);
	if (sock != -1){
		list_operation(GET_MUTEX, &mutr, NULL, NULL,instrument_id);
		if (check_status_ok(sock,mutr)==0){
			printf("Device status: not OK!\n");
			return 0;
		}

	}
	else{
		timeout.tv_sec = 5;
		timeout.tv_usec = 5;
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(LPCPORT);
		//TODO:implement getting ip from system call using instrument id
		//servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		servaddr.sin_addr.s_addr = inet_addr(ipTable[instrument_id]);
	
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			perror("epics_TCP_connect:socket failed");
			return 0;
		}

		//if(setsockopt (sock,SOL_SOCKET,SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
			//perror("setsockopt failed\n");

		//if(setsockopt (sock,SOL_SOCKET,SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
			//perror("setsockopt failed\n");

		if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0){
			perror("epics_TCP_connect:connection failed\n");
			return 0;
		}
		list_operation(CREATE_NODE, NULL, &sock, &last,instrument_id);
		if (check_status_ok(last->sock,last->mutex)==0){
			printf("Device status: not OK!\n");
			list_operation(REMOVE_NODE,NULL,NULL,NULL,instrument_id);
			return 0;
		}
	}
	
	return 1;
}

static int comm_talk(command_header ask,int size, command_header *answer, int instrument_id){
	int ret_val = 1;
	int sock;
	fd_set rfds;
	fd_set wfds;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	epicsMutexId mutex;
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 5;
	list_operation(GET_SOCK,NULL,&sock,NULL,instrument_id);
	if(sock == -1){
		perror("Device disconnected");
		return 0;
	}
	
	int list_ret = list_operation(GET_MUTEX, &mutex, NULL, NULL,instrument_id);
	if(list_ret == 0){
		perror("No device found");
		return 0;
	}
	
	printf("sock %d, list_ret %d \n",sock,list_ret);
	
	epicsMutexLock(mutex);
	
	FD_SET(sock, &rfds);
	FD_SET(sock, &wfds);
	if(select(sock+1,NULL,&wfds,NULL,&timeout)==-1){
		perror("select error");
		ret_val = 0;
	}
	else if(FD_ISSET(sock,&wfds)){
		if(send(sock, &ask, size, 0)<=0){
			perror("epics_TCP_get:message not sent");
			ret_val=0;
			goto end;
		}
	}
	else{
		perror("timeout:send");
		ret_val = 0;
		goto end;
	}
	
	if(select(sock+1,&rfds,NULL,NULL,&timeout)==-1){
		perror("select error");
		ret_val = 0;
	}
	else if(FD_ISSET(sock,&rfds)){
		if(recv(sock,answer,sizeof(answer),0)<=0){
			perror("epics_TCP_get:message not recv");
			ret_val=0;
			goto end;
		}
	}
	else{
		perror("timeout:recv");
		ret_val = 0;
		goto end;
	}
	end:
	FD_CLR(sock,&wfds);
	FD_CLR(sock,&rfds);
	epicsMutexUnlock(mutex);
	if (ret_val)
		printf("message received:%d %d %d\n",answer->command,answer->size,answer->p[0]);
	return ret_val;
}
	
//get message from server
static int get_read(epicsUInt8 **buf, int instrument_id, int variable, int numbytes){
	command_header ask,answer;
	int size,ret_val;
	size = var_read_command_ask(&ask);	
	ask.p[0] = (unsigned char)variable;
	ret_val = comm_talk(ask,size,&answer,instrument_id);
	if (ret_val){
		*buf = (epicsUInt8*)malloc(sizeof(epicsUInt8)*numbytes);
		memcpy(*buf,answer.p,numbytes);
	}
	if (!ret_val)
		list_operation(REMOVE_NODE,NULL,NULL,NULL,instrument_id);
	
	return ret_val;
}

static int get_write(epicsUInt8 **buf, int instrument_id, int variables, int numbytes){
	command_header ask,answer;
	int  size, ret_val,i;
	size = var_write_command_ask(&ask,numbytes);
	ask.p[0] = (unsigned char)variables;
	for(i=0;i<numbytes;i++)
		ask.p[i+1] = (unsigned char)(*buf)[i];
	
	ret_val = comm_talk(ask,size,&answer,instrument_id);
	if (!ret_val)
		list_operation(REMOVE_NODE,NULL,NULL,NULL,instrument_id);
	return ret_val;
}

static int get_operation(epicsUInt8 **buf, int instrument_id, int variable,enum operation op,int numbytes){
	
	switch(op){
		case OP_READ_BI:
			return get_read(buf,instrument_id,variable,numbytes);
			break;
		case OP_READ_AI:
			return get_read(buf,instrument_id,variable,numbytes);
			break;
		case OP_READ_MBBI:
			return get_read(buf,instrument_id,variable,numbytes);
			break;
		case OP_WRITE_MBBO:
			return get_write(buf,instrument_id,variable,numbytes);	
			break;
		case OP_WRITE_BO:
			return get_write(buf,instrument_id,variable,numbytes);	
			break;
		case OP_WRITE_AO:
			return get_write(buf,instrument_id,variable,numbytes);	
			break;
	}
	return 0;
}

//multiplex right operation
int epics_TCP_do(epicsUInt8 **buf, int instrument_id, int variable, enum operation op, int numbytes){

	return get_operation(buf,instrument_id,variable,op,numbytes);
}
