#include <stdio.h>
#include <epicsStdio.h>
#include <epicsStdlib.h>
#include <epicsString.h>
#include <epicsExport.h>
#include <epicsMutex.h>
#include <drvSup.h>
#include <iocsh.h>

typedef union {
	epicsUInt8 c[4];
	float f;
} u;	
