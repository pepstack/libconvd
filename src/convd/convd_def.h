/**
 * @file convd_def.h
 * @brief libconvd public definitions.
 *
 * @author cheungmine@qq.com
 * @version
 * @date
 * @note
 * @since
 */
#ifndef CONVD_DEF_H_PUBLIC
#define CONVD_DEF_H_PUBLIC

#ifdef    __cplusplus
extern "C" {
#endif

#include <common/unitypes.h>


#if defined(CONVD_DLL)
/* win32 dynamic dll */
# ifdef CONVD_EXPORTS
#   define CONVD_API __declspec(dllexport)
# else
#   define CONVD_API __declspec(dllimport)
# endif
#else
/* static lib or linux so */
# define CONVD_API  extern
#endif

typedef int CONVD_BOOL;

#define CONVD_TRUE    1
#define CONVD_FALSE   0


typedef int CONVD_RESULT;

/**
 * success result code
 */
#define CONVD_NOERROR     0

/**
 * failure result codes 
 */
#define CONVD_ERR_FROMCODE       1
#define CONVD_ERR_TOCODE         2
#define CONVD_ERR_SUFFIX         3
#define CONVD_ERR_ENCODING       4
#define CONVD_ERR_INSUFF         5
#define CONVD_ERR_ICONV        (-1)
#define CONVD_ERR_FILE         (-2)


typedef struct _conv_descriptor_t    *convd_t;
typedef struct _xml_attrs_t          *xml_attrs;


/**
 * max length for charset encoding including end null char.
 * Lists the supported encodings:
 *   $ iconv --list
 */
#define CVD_ENCODING_LEN_MAX  64


typedef enum {
    /**
     * No suffix string appended to tocode.
     */
    CVD_SUFFIX_NONE =     0,

    /**
     * The string "//IGNORE" will be appended to tocode,
     * This means that characters that cannot be represented in the target character
     * set will be silently discarded.
     */
    CVD_SUFFIX_IGNORE =   1,

    /**
     * The string "//TRANSLIT" will be appended to tocode:
     * This means that when a character cannot be represented in the target character
     * set, it can be approximated through one or several similarly looking characters.
     */
    CVD_SUFFIX_TRANSLIT  = 2
} CONVD_SUFFIX_MODE;


/**
 * Unicode Character Set
 */
typedef enum {
    /**
     * Byte Order Mark not found
     */
    UCS_NONE_BOM     = 0,

    /**
     * UTF-8: 'EF BB BF'
     *   8-bit UCS Transformation Format with BOM header
     */
    UCS_UTF8_BOM     = 1,

    /**
     * UTF-16BE: 'FE FF'
     *   16-bit UCS Transformation Format, big-endian byte order
     */
    UCS_2BE_BOM     = 2,

    /**
     * UTF-16LE: 'FF FE'
     *   16-bit UCS Transformation Format, little-endian byte order
     */
    UCS_2LE_BOM     = 3,

    /**
     * UTF-32BE: '00 00 FE FF'
     *   32-bit UCS Transformation Format, big-endian byte order
     */
    UCS_4BE_BOM     = 4,

    /**
     * UTF-32LE: 'FF FE 00 00'
     *   32-bit UCS Transformation Format, little-endian byte order
     */
    UCS_4LE_BOM     = 5,

    /**
     * UTF-8?: 'EF BB ?'
     *   Ask next one byte to determine if it is UTF_UTF_8BOM
     */
    UCS_UTF8_BOM_ASK = 6
} CONVD_UCS_BOM;


typedef struct
{
    size_t blen;
    char *bufp;
} conv_buf_t;


typedef struct
{
    CONVD_UCS_BOM bom;
    char version[16];
    char encoding[CVD_ENCODING_LEN_MAX];
} conv_xmlhead_t;

#ifdef    __cplusplus
}
#endif

#endif /* CONVD_DEF_H_PUBLIC */