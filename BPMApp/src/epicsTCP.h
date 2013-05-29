#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include "../../Api-para-Protocolo-de-Controle-de-Baixo-n-vel/pcbnAPI.h"
#include "epicsMutex.h"

#define LPCPORT 6791
#define READ_OP 0
#define WRITE_OP 1

enum operation {
	OP_READ_BI,
	OP_READ_AI,
	OP_WRITE_BO,
	OP_WRITE_AO
};

int epics_TCP_connect(int instrument_id);

int epics_TCP_do(int sock, epicsUInt8 *buf, int instrument_id, int variable, enum operation){
