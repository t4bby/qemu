#ifndef QEMU_SMBIOS_PATCHER_H
#define QEMU_SMBIOS_PATCHER_H

#include "contrib/smbios-parser/smbios.h"

/*
 * SMBIOS Patcher
 * Copyright (C) 2026 Tabby <me@t4bby.dev>
 */

// Offsets within Type 1 formatted section (System Information)
// See SMBIOS spec:
enum {
    SYS_OFF_MANUFACTURER = 4, // uint8 string index
    SYS_OFF_PRODUCT = 5,      // uint8 string index
    SYS_OFF_VERSION = 6,      // uint8 string index
    SYS_OFF_SERIAL = 7,       // uint8 string index
    SYS_OFF_UUID = 8,         // 16 bytes UUID
    SYS_OFF_SKU = 25,         // uint8 string index (SMBIOS 2.4+)
    SYS_OFF_FAMILY = 26,      // uint8 string index (SMBIOS 2.4+)
};

enum {
    PROC_OFF_FAMILY = 0x16, // uint8 ProcessorFamily
};

// Offsets within Type 17 formatted section (SMBIOS 2.7+/3.x)
// See SMBIOS spec:
// Size at 0x0C,
// Speed at 0x15,
// ExtendedSize at 0x1C,
// ConfiguredClockSpeed at 0x20
enum {
    MEM_OFF_SIZE = 12, // uint16 GB (0x7FFF means check ExtendedSize)
    MEM_OFF_SPEED = 21, // uint16 MT/s
    MEM_OFF_EXTSIZE = 28, // uint32 MB (when Size == 0x7FFF)
    MEM_OFF_CONF_SPEED = 32, // uint16 MT/s
    MEM_OFF_TYPE = 18, // uint8 MemoryType (DDR generation enum)
};

// Offsets within Type 20 formatted section (Memory Device Mapped Address)
// See SMBIOS spec:
// StartingAddress at 0x04,
// EndingAddress at 0x08,
// MemoryDeviceHandle at 0x0C
// ExtendedStartingAddress at 0x13 (SMBIOS 2.7+),
// ExtendedEndingAddress at 0x1B (SMBIOS 2.7+)
enum {
    MEMMAP_OFF_STARTING_ADDR = 4,  // uint32 KB
    MEMMAP_OFF_ENDING_ADDR = 8,    // uint32 KB
    MEMMAP_OFF_DEVICE_HANDLE = 12, // uint16 handle
    MEMMAP_OFF_EXT_STARTING_ADDR = 19, // uint64 bytes (SMBIOS 2.7+)
    MEMMAP_OFF_EXT_ENDING_ADDR = 27, // uint64 bytes (SMBIOS 2.7+)
};

extern struct ParserContext parser;

void smbios_patcher_apply(const uint8_t *buffer, size_t length, unsigned long type, QemuOpts *opts, Error **errp);

void smbios_patcher_patch_entry_string(const struct Entry *entry, uint8_t string_index,
                         const char *new_value);

void smbios_patcher_parse_uuid(const char *uuid_str, uint8_t *uuid_out, Error **errp);

#endif
