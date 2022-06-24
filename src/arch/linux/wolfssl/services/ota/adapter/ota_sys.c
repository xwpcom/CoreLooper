/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "ota_sys.h"

#include <sys/time.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*****************************************************************************/
/* Local Definitions ( Constant and Macro)                                  */
/*****************************************************************************/

struct countdown_tmr_t
{
    uint64_t time_end;
};

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/

int ota_log_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    static uint8_t _a_ = 0;
    if(_a_ == 0)
    {
        _a_ = 1;
        remove("ota.log");
    }

    FILE *logfd;
    logfd = fopen("ota.log", "ab+");
    char temp[512] = {0};

    va_list args2;
    va_start(args2, format);

    vsprintf(temp, format, args2);
    va_end(args2);

    fwrite(temp, sizeof(char), strlen(temp), logfd);
    fclose(logfd);

    return OTA_OK;
}

void ota_calculate_segment_range(uint32_t package_now, uint32_t *start, uint32_t *end)
{
    static uint32_t every_download_size = 700 * 1024;
    package_now > 0 ? (*start = package_now + 1) : (*start = 0);
    *end = package_now + every_download_size;
}

void ota_deinit_free(void *p)
{
    if(p != NULL)
    {
        ota_free(p);
    }
}

int ota_atoi(const char *nptr)
{
    return atoi(nptr);
}

uint8_t ota_scene_recall(void *user_context)
{
    // ota_context* ctx = (ota_context*)user_context;
    return OTA_OK;
}

uint8_t ota_scene_store(void *user_context)
{
    return OTA_OK;
}

void ota_hex2str(const char *src_hex, char *Dst_str, int hex_len)
{
    int i;
    char tmp[3] = {0};
    for(i = 0; i < hex_len; i++)
    {
        sprintf(tmp, "%02x", (unsigned char)src_hex[i]);
        memcpy(&Dst_str[i * 2], tmp, 2);
    }
    return;
}
