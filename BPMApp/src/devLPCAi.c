#include <stdlib.h>
#include <epicsExport.h>
#include <dbAccess.h>
#include <devSup.h>
#include <recGbl.h>
#include <callback.h>
#include "epicsTCP.h"
#include <biRecord.h>

static long init_record(biRecord *pao);
static long read_ai(biRecord *pao);

struct LPCState {
	int sock;
	unsigned long inp;
};

struct {
	long num;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN read_ai;
	DEVSUPFUN special_lincov;
}devLPCBi = {
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	read_ai,
	NULL
};
epicsExportAddress(dset,devLPCBi);

static long init_record(biRecord *pao){
	struct LPCState *priv;
	unsigned long start;

	priv=malloc(sizeof(struct LPCState));
	if(!priv){
		recGblRecordError(S_db_noMemory, (void*)pao, "devBiTimebase failed to allocate private struct");
		return S_db_noMemory;
	}
	
	
	priv->sock=epics_TCP_connect(instrument_id);
	recGBInitConstantLink(&pao->inp,DBF_ULONG,&start);
	
	priv->inp=start;
	pao->dpvt=priv;
	
	return 0;
}

static long read_bi(biRecord *pao){
	struct LPCState *priv = pao->dpvt;
	int status;
	epicsUInt8 buf;
	status = epics_TCP_do(priv->sock,&buf,(int)start->inp,0,OP_READ_BI);
	
	return 0;
}
