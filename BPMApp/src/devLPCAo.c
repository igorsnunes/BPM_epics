#include <stdlib.h>
#include <stdio.h>
#include <epicsExport.h>
#include <dbAccess.h>
#include <devSup.h>
#include <recGbl.h>
#include <aoRecord.h>
#include <stdlib.h>
#include "epicsTCP.h"

typedef union {
	epicsUInt8 c[4];
	float f;
} u;
	
static long init_record_ao(aoRecord *record);
static long write_ao(aoRecord *record);

struct LPCState {
	int status;
	int sock;
	unsigned long instr_id;
};

struct {
	long num;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_record_ao;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN write_ao;
	DEVSUPFUN special_linconv;
} devLPCAo = {
	6,
	//5,
	NULL,
	NULL,
	init_record_ao,
	NULL,
	write_ao,
	NULL
};
epicsExportAddress(dset, devLPCAo);

static long init_record_ao(aoRecord *pao){
	int sock = 0;
	struct LPCState *priv;
	priv=malloc(sizeof (*priv) );
	if(!priv)
		return S_db_noMemory;

	int instrument_id;
	if(sscanf(pao->out.value.instio.string, "%d", &instrument_id)!=1)
		return S_db_errArg;
	
	printf("instrument_id: %d\n",instrument_id);
	
	priv->instr_id = instrument_id;
	if (epics_TCP_connect(instrument_id,&sock,1)==0){
		priv->status = 0;
		pao->dpvt = priv;
		return S_dev_noDeviceFound;
	}else {
		printf("sock: %d\n",sock);
		priv->status = 1;
		priv->sock = sock; 
	}
	pao->dpvt = priv;
	return 0;
}

static long write_ao(aoRecord *pao){
	struct LPCState *priv = pao->dpvt;
	//epicsUInt32 value = pao->rval;
	u rval;
	int i;
	epicsUInt8 *buff = NULL;
	if (priv->status){
		buff = (epicsUInt8*)malloc(sizeof(epicsUInt8)*4);
		if(!buff)
			return S_db_noMemory;
		rval.f = pao->rval;
		memcpy(buff,&rval.c,4);
		priv->status = epics_TCP_do(priv->sock,&buff,priv->instr_id,1,OP_WRITE_AO);
	
		if (buff)
			free(buff);
	}
	 else
		priv->status = epics_TCP_connect(priv->instr_id,&priv->sock,0);

	return 0;
}
