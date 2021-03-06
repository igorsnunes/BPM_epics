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
	int variable;
	int numbytes;
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
	int variable;
	int numbytes;
	struct LPCState *priv;
	priv=malloc(sizeof (*priv) );
	if(!priv)
		return S_db_noMemory;

	int instrument_id;
	if(sscanf(pao->out.value.instio.string, "%d.%d.%d", &instrument_id,&variable,&numbytes)!=3)
		return S_db_errArg;
	
	priv->instr_id = instrument_id;
	priv->variable = variable;
	priv->numbytes = numbytes;
	
	if (epics_TCP_connect(instrument_id)==0){
		priv->status = 0;
		pao->dpvt = priv;
	}else {
		priv->status = 1;
		priv->sock = sock; 
	}
	pao->dpvt = priv;
	return 0;
}

static long write_ao(aoRecord *pao){
	struct LPCState *priv = pao->dpvt;
	u rval;
	int i;
	epicsUInt8 *buff = NULL;
	if (priv->status){
		buff = (epicsUInt8*)malloc(sizeof(epicsUInt8)*priv->numbytes);
		if(!buff)
			return S_db_noMemory;
		rval.f = pao->rval;
		memcpy(buff,&rval.c,priv->numbytes);
		priv->status = epics_TCP_do(&buff,priv->instr_id,priv->variable,OP_WRITE_AO,priv->numbytes);
	
		if (buff)
			free(buff);
	}
	 else
		priv->status = epics_TCP_connect(priv->instr_id);

	return 0;
}
