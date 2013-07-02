#include <stdlib.h>
#include <stdio.h>
#include <epicsExport.h>
#include <dbAccess.h>
#include <devSup.h>
#include <recGbl.h>
#include <biRecord.h>
#include <stdlib.h>
#include "epicsTCP.h"


typedef union {
	epicsUInt8 c[4];
	unsigned long f;
} u;
static long init_record_bi(biRecord *record);
static long read_bi(biRecord *record);

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
	DEVSUPFUN init_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN read_bi;
} devLPCBi = {
	5,
	NULL,
	NULL,
	init_record_bi,
	NULL,
	read_bi,
};
epicsExportAddress(dset,devLPCBi);

static long init_record_bi(biRecord *pao){
	int variable;
	int sock = 0;
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

static long read_bi(biRecord *pao){
	struct LPCState *priv = pao->dpvt;
	u rval;
	epicsUInt8 *buf = NULL;
	if (priv->status){
		priv->status = epics_TCP_do(&buf,priv->instr_id,priv->variable,OP_READ_BI,priv->numbytes);
		if (priv->status){
			memcpy(&rval.c,buf,priv->numbytes);
			pao->rval = rval.f;
			free(buf);
		}
	} else
		priv->status = epics_TCP_connect(priv->instr_id);

	return 0;
}
