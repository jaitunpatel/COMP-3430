
// Header file already provided in lab 4
#ifndef DIRINFO_H
#define DIRINFO_H

#include <inttypes.h>

#pragma pack(push)
#pragma pack(1)
struct DirInfo {
    char dir_name[12];
    uint8_t dir_attr;
    uint8_t dir_ntres;
    uint8_t dir_crt_time_tenth;
    uint16_t dir_crt_time;
    uint16_t dir_crt_date;
    uint16_t dir_last_access_time;
    uint16_t dir_first_cluster_hi;
    uint16_t dir_wrt_time;
    uint16_t dir_wrt_date;
    uint16_t dir_first_cluster_lo;
    uint32_t dir_file_size;
};
#pragma pack(pop)
#endif
