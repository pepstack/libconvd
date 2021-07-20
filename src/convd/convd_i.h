/**
 * convd_i.h
 */
#ifndef _CONVD_I_H_
#define _CONVD_I_H_

#ifdef _MSC_VER
# pragma warning (disable : 4996)
#endif

#ifdef    __cplusplus
extern "C" {
#endif

static const char LIBNAME[] = "convd";
static const char LIBVERSION[] = "0.0.1";

#include <common/misc.h>
#include <common/emerglog.h>
#include <common/timeut.h>
#include <common/refcobject.h>
#include <common/bo.h>

#include <iconv.h>

#include "convd_api.h"


#define CONVD_ERROR_ICONV    ((iconv_t)(-1))
#define CONVD_ERROR_SIZE     ((size_t)(-1))

#define CONV_FINISHED        0

#define CONV_LINE_MAXSIZE    ((ub4) 65536)


typedef struct _conv_descriptor_t
{
    iconv_t cd;
    CONVD_SUFFIX_MODE suffix;
    int tocodeat;
    char codebuf[0];
} conv_descriptor_t;


typedef struct _conv_position_t
{
    filehandle_t textfd;
    sb8 offset;

    convd_t cvd;
    CONVD_UCS_BOM bom;

    char *inputbuf;
    char *outputbuf;

    ub4 linesize;
    char linebuf[0];
} conv_position_t, *convpos_t;


STATIC_INLINE int strcmp_caseign(const char *Astr, const char *Bstr)
{
    #ifdef _MSC_VER
        return stricmp(Astr, Bstr);
    #else
        return strcasecmp(Astr, Bstr);
    #endif
}

#ifdef    __cplusplus
}
#endif

#endif /* _CONVD_I_H_ */