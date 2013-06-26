#include <stdlib.h>
#include <stdio.h>
#include <epicsExport.h>
#include <dbAccess.h>
#include <devSup.h>
#include <recGbl.h>
#include <mbboRecord.h>
#include <stdlib.h>
#include "epicsTCP.h"

typedef union {
	epicsUInt8 c[4];
	unsigned long f;
} u;
static long init_record_mbbo(mbboRecord *record);
static long write_mbbo(mbboRecord *record);

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
	DEVSUPFUN write_mbbo;
} devLPCMbbo = {
	5,
	NULL,
	NULL,
	init_record_mbbo,
	NULL,
	write_mbbo,
};
epicsExportAddress(dset,devLPCMbbo);

static long init_record_mbbo(mbboRecord *pao){
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

	if (epics_TCP_connect(instrument_id,&sock,1)==0){
		priv->status = 0;
		pao->dpvt = priv;
		return S_dev_noDeviceFound;
	}else {
		priv->status = 1;
		priv->sock = sock; 
	}

	pao->dpvt = priv;
	return 0;
}

static long write_mbbo(mbboRecord *pao){
	struct LPCState *priv = pao->dpvt;
	epicsUInt8 *buf = NULL;
	u rval;
	if (priv->status){
		buf = (epicsUInt8*)malloc(sizeof(epicsUInt8)*((pao->nobt/8)+1));
		if (!buf)
			return S_db_noMemory;
		rval.f = pao->rval;
		memcpy(buf,&rval.c,((pao->nobt/8)+1));
		priv->status = epics_TCP_do(priv->sock,&buf,priv->instr_id,priv->variable,OP_WRITE_MBBO,((pao->nobt/8)+1));
		if (buf)//TODO: check it!
			free(buf);
	} else
		priv->status = epics_TCP_connect(priv->instr_id,&priv->sock,0);

	return 0;
}
