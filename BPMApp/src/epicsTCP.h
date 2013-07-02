#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include "submodules/pcbnAPI.h"
#include "epicsMutex.h"

#include <epicsStdio.h>
#include <epicsStdlib.h>
#include <epicsString.h>
#include <epicsExport.h>
#include <epicsMutex.h>
#include <drvSup.h>
#include <iocsh.h>


#define LPCPORT 6791
#define READ_OP 0
#define WRITE_OP 1

typedef struct node{
	int sock;
	int device_id;
	epicsMutexId mutex;
	struct node *next;
}Node;

enum ListOperation {
	GET_MUTEX,
	GET_SOCK,
	CREATE_NODE,
	REMOVE_NODE
};

typedef struct pnode{
	int nnode;
	struct node *first;
}pNode;

enum operation {
	OP_READ_BI,
	OP_READ_AI,
	OP_WRITE_BO,
	OP_WRITE_AO,
	OP_READ_MBBI,
	OP_WRITE_MBBO
};

int init_pNode(void);
int epics_TCP_connect(int instrument_id);
int epics_TCP_do(epicsUInt8 **buf, int instrument_id, int variable, enum operation op,int numbytes);
