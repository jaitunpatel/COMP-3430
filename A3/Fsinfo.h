
// Header file already provided in lab 4
#ifndef FSINFO_H
#define FSINFO_H

#include <inttypes.h>

#pragma pack(push)
#pragma pack(1)
struct Fsinfo {
    uint32_t FSI_LeadSig;
    char FSI_Reserved1[480];
    uint32_t FSI_StrucSig;
    uint32_t FSI_Free_Count;
};
#pragma pack(pop)
#endif
