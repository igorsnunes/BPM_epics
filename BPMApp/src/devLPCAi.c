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
	int variable;
	int numbytes;
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
	int variable;
	int numbytes;
	struct LPCState *priv;
	priv=malloc(sizeof (*priv) );
	if(!priv)
		return S_db_noMemory;

	int instrument_id;
	if(sscanf(pao->inp.value.instio.string, "%d.%d.%d", &instrument_id,&variable,&numbytes)!=3)
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

static long read_ai(aiRecord *pao){
	struct LPCState *priv = pao->dpvt;
	u rval;
	epicsUInt8 *buff = NULL;
	if (priv->status){
		priv->status = epics_TCP_do(&buff,priv->instr_id,priv->variable,OP_READ_AI,priv->numbytes);
		if (priv->status){
			memcpy(&rval.c,buff,priv->numbytes);
			pao->rval = rval.f;
			free(buff);
		}
	} else
		priv->status = epics_TCP_connect(priv->instr_id);

	return 0;
}
