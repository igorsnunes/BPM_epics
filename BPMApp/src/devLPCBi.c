#include <stdlib.h>
#include <stdio.h>
#include <epicsExport.h>
#include <dbAccess.h>
#include <devSup.h>
#include <recGbl.h>
#include <biRecord.h>
#include <stdlib.h>
#include "epicsTCP.h"


static long init_record_bi(biRecord *record);
static long read_bi(biRecord *record);

struct LPCState {
	int status;
	int sock;
	int variable;
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
	struct LPCState *priv;
	priv=malloc(sizeof (*priv) );
	if(!priv)
		return S_db_noMemory;

	int instrument_id;
	
	if(sscanf(pao->inp.value.instio.string, "%d.%d", &instrument_id,&variable)!=2)
		return S_db_errArg;
	
	priv->instr_id = instrument_id;
	priv->variable = variable;

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

static long read_bi(biRecord *pao){
	struct LPCState *priv = pao->dpvt;
	epicsUInt8 *buf = NULL;
	if (priv->status){
		priv->status = epics_TCP_do(priv->sock,&buf,priv->instr_id,priv->variable,OP_READ_BI);
		if (priv->status){
			pao->rval = buf[0];
		}
		if (!buf)
			free(buf);
	} else
		priv->status = epics_TCP_connect(priv->instr_id,&priv->sock,0);

	return 0;
}
