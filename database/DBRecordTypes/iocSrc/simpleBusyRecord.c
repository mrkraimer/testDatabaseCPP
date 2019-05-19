/*************************************************************************\
* Copyright (c) 2008 UChicago Argonne LLC, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/

/* Revision-Id: anj@aps.anl.gov-20131120222110-3o0wgh76u652ad4e */

/* recLongin.c - Record Support Routines for Longin records */
/*
 *      Author: 	Janet Anderson
 *      Date:   	9/23/91
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "dbDefs.h"
#include "epicsPrint.h"
#include "alarm.h"
#include "dbAccess.h"
#include "dbEvent.h"
#include "dbFldTypes.h"
#include "devSup.h"
#include "errMdef.h"
#include "recSup.h"
#include "recGbl.h"
#include "menuYesNo.h"

#define GEN_SIZE_OFFSET
#include "simpleBusyRecord.h"
#undef  GEN_SIZE_OFFSET
#include "epicsExport.h"

/* Create RSET - Record Support Entry Table*/
#define report NULL
#define initialize NULL
static long init_record(struct dbCommon *, int);
static long process(struct dbCommon *);
static long special(struct dbAddr *paddr, int after);
#define get_value NULL
#define cvt_dbaddr NULL
#define get_array_info NULL
#define put_array_info NULL
#define get_units NULL
#define get_precision NULL
#define get_enum_str NULL
#define get_enum_strs NULL
#define put_enum_str NULL
#define get_graphic_double NULL
#define get_control_double NULL
#define get_alarm_double NULL


rset simpleBusyRSET={
	RSETNUMBER,
	report,
	initialize,
	init_record,
	process,
	special,
	get_value,
	cvt_dbaddr,
	get_array_info,
	put_array_info,
	get_units,
	get_precision,
	get_enum_str,
	get_enum_strs,
	put_enum_str,
	get_graphic_double,
	get_control_double,
	get_alarm_double
};
epicsExportAddress(rset,simpleBusyRSET);


static long init_record(struct dbCommon *pcommon, int pass)
{
    struct simpleBusyRecord *prec = (simpleBusyRecord *)pcommon;
    prec->val = 0;
    prec->udf = FALSE;
    recGblResetAlarms(prec);
    return(0);
}

static long process(struct dbCommon *pcommon)
{
    struct simpleBusyRecord *prec = (simpleBusyRecord *)pcommon;
    unsigned short monitor_mask = DBE_VALUE;
    recGblGetTimeStamp(prec);
    db_post_events(prec, &prec->val, monitor_mask);
    if(prec->val!=0) {
        prec->pact = TRUE;
        return 0;
    }
    recGblFwdLink(prec);
    prec->pact=FALSE;
    return 0;
}

static long special(struct dbAddr *paddr, int after)
{
    struct dbCommon *prec = paddr->precord;
    if(after==0) return 0;
    if(prec->pact==FALSE) return 0;
    prec->rset->process(prec);
    return 0;
}
