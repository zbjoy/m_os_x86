#ifndef LOADER_H
#define LOADER_H

#include "comm/boot_info.h"
#include "comm/types.h"
#include "comm/cpu_instr.h"

void protect_mode_entry(void);

typedef struct SMAP_entry {
    uint32_t BaseL;
    uint32_t BaseH;
    uint32_t LenghtL;
    uint32_t LenghtH;
    uint32_t Type;  // entry Type, 值为1时 表明 为我们可用的RAM空间
    uint32_t ACPI; // extended, bit0 = 1 时 表明此条目应当被忽略
} __attribute__((packed)) SMAP_entry_t;

#endif /* LOADER_H */
