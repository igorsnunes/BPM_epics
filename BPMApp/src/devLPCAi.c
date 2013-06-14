#include <stdlib.h>
#include <stdio.h>
#include <epicsExport.h>
#include <dbAccess.h>
#include <devSup.h>
#include <recGbl.h>
#include <aiRecord.h>
#include <stdlib.h>
#include "epicsTCP.h"

typedef union {
	epicsUInt8 c[4];
	float f;
} u;
	
static long init_record_ai(aiRecord *record);
static long read_ai(aiRecord *record);

struct LPCState {
	int status;
	int sock;
	unsigned long instr_id;
};

struct {
	long num;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_record_ai;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN read_ai;
} devLPCAi = {
	6,
	NULL,
	NULL,
	init_record_ai,
	NULL,
	read_ai,
	NULL
};
epicsExportAddress(dset, devLPCAi);

static long init_record_ai(aiRecord *pao){
	int sock = 0;
	struct LPCState *priv;
	priv=malloc(sizeof (*priv) );
	if(!priv)
		return S_db_noMemory;

	int instrument_id;
	if(sscanf(pao->inp.value.instio.string, "%d", &instrument_id)!=1)
		return S_db_errArg;
	
	printf("instrument_id: %d\n",instrument_id);
	
	if (epics_TCP_connect(instrument_id,&sock,1)==0){
		priv->status = 0;
		pao->dpvt = priv;
		return S_dev_noDeviceFound;
	}else {
		printf("sock: %d\n",sock);
		priv->status = 1;
		priv->sock = sock; 
	}
	priv->instr_id = instrument_id;
	pao->dpvt = priv;
	return 0;
}

static long read_ai(aiRecord *pao){
	struct LPCState *priv = pao->dpvt;
	u rval;
	epicsUInt8 *buff = NULL;
	if (priv->status){
		printf("before do\n");
		priv->status = epics_TCP_do(priv->sock,&buff,priv->instr_id,1,OP_READ_AI);
		printf("after do\n");
		if (priv->status){
			rval.c[0] = buff[0];
			rval.c[1] = buff[1];
			rval.c[2] = buff[2];
			rval.c[3] = buff[3];
			printf("float value:%f\n", rval.f);
			pao->rval = rval.f;
		}
		if (buff)
			free(buff);
	} else
		priv->status = epics_TCP_connect(priv->instr_id,&priv->sock,0);

	return 0;
}
