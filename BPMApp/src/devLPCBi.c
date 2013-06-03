#include <stdlib.h>
#include <stdio.h>
#include <epicsExport.h>
#include <dbAccess.h>
#include <devSup.h>
#include <recGbl.h>
#include <biRecord.h>
#include <stdlib.h>

//#include "../../Api-para-Protocolo-de-Controle-de-Baixo-n-vel/pcbnAPI.h"
#include "epicsTCP.h"


static long init_record_bi(biRecord *record);
static long read_bi(biRecord *record);

struct LPCState {
	int sock;
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
	struct LPCState *priv;

	priv=malloc(sizeof(struct LPCState));
	if(!priv){
		recGblRecordError(S_db_noMemory, (void*)pao, "devBiTimebase failed to allocate private struct");
		return S_db_noMemory;
	}
	int instrument_id;
	if(sscanf(pao->inp.value.instio.string, "%d", &instrument_id)!=1)
		return S_db_errArg;

	
	priv->sock = epics_TCP_connect(instrument_id);
	priv->instr_id = instrument_id;
	pao->dpvt = priv;
	
	return 0;
}

static long read_bi(biRecord *pao){
	struct LPCState *priv = pao->dpvt;
	int status;
	epicsUInt8 buf;
	status = epics_TCP_do(priv->sock,&buf,priv->instr_id,0,OP_READ_BI);
	
	return 0;
}
