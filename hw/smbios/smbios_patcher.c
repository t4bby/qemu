/*
* SMBIOS Patcher
 * Copyright (C) 2026 Tabby <me@t4bby.dev>
 */

#include "qemu/osdep.h"
#include "hw/firmware/smbios_patcher.h"
#include "qapi/error.h"
#include "qemu/option.h"

// #define DEBUG_SMBIOS_PATCHER

#ifdef DEBUG_SMBIOS_PATCHER
#define DPRINTF(fmt, ...) \
do { printf("smbios_patcher: " fmt , ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) do {} while(0)
#endif

struct ParserContext parser;

void smbios_patcher_apply(const uint8_t *buffer, const size_t length,
                          const unsigned long type, QemuOpts *opts, Error **errp) {
    if (smbios_initialize(&parser, (uint8_t *) buffer, length, SMBIOS_ANY) !=
        SMBERR_OK) {
        error_setg(errp, "Failed to initialize SMBIOS parser");
        return;
    }

    const struct Entry *entry = NULL;
    int err;
    while ((err = smbios_next(&parser, &entry)) == SMBERR_OK) {
        switch (entry->type) {
            case TYPE_BIOS_INFO:
                if (type != TYPE_BIOS_INFO)
                    break;
                smbios_patcher_patch_entry_string(entry, entry->data.bios_info.Vendor_,
                                                  qemu_opt_get(opts, "vendor"));
                smbios_patcher_patch_entry_string(entry, entry->data.bios_info.BIOSVersion_,
                                                    qemu_opt_get(opts, "version"));
                smbios_patcher_patch_entry_string(entry, entry->data.bios_info.BIOSReleaseDate_,
                                                    qemu_opt_get(opts, "date"));

                break;
            case TYPE_SYSTEM_INFO:
                if (type != TYPE_SYSTEM_INFO)
                    break;

                smbios_patcher_patch_entry_string(entry, entry->data.system_info.Manufacturer_,
                                                  qemu_opt_get(opts, "manufacturer"));
                smbios_patcher_patch_entry_string(entry, entry->data.system_info.ProductName_,
                                                  qemu_opt_get(opts, "product"));
                smbios_patcher_patch_entry_string(entry, entry->data.system_info.Version_,
                                                  qemu_opt_get(opts, "version"));
                smbios_patcher_patch_entry_string(entry, entry->data.system_info.SerialNumber_,
                                                  qemu_opt_get(opts, "serial"));
                smbios_patcher_patch_entry_string(entry, entry->data.system_info.SKUNumber_,
                                                  qemu_opt_get(opts, "sku"));
                smbios_patcher_patch_entry_string(entry, entry->data.system_info.Family_,
                                                  qemu_opt_get(opts, "family"));

                const char *new_uuid_str = qemu_opt_get(opts, "uuid");
                if (new_uuid_str) {
                    uint8_t uuid[16];
                    smbios_patcher_parse_uuid(new_uuid_str, uuid, errp);

                    uint8_t *uuid_ptr = (uint8_t *)entry->rawdata + SYS_OFF_UUID;
                    memcpy(uuid_ptr, uuid, 16);
                }

                break;
            case TYPE_BASEBOARD_INFO:
                if (type != TYPE_BASEBOARD_INFO)
                    break;
                smbios_patcher_patch_entry_string(entry, entry->data.baseboard_info.Manufacturer_,
                                                  qemu_opt_get(opts, "manufacturer"));
                smbios_patcher_patch_entry_string(entry, entry->data.baseboard_info.Product_,
                                                  qemu_opt_get(opts, "product"));
                smbios_patcher_patch_entry_string(entry, entry->data.baseboard_info.Version_,
                                                  qemu_opt_get(opts, "version"));
                smbios_patcher_patch_entry_string(entry, entry->data.baseboard_info.SerialNumber_,
                                                  qemu_opt_get(opts, "serial"));
                smbios_patcher_patch_entry_string(entry, entry->data.baseboard_info.AssetTag_,
                                                  qemu_opt_get(opts, "asset"));
                smbios_patcher_patch_entry_string(entry, entry->data.baseboard_info.LocationInChassis_,
                                                  qemu_opt_get(opts, "location"));
                break;
            case TYPE_SYSTEM_ENCLOSURE:
                if (type != TYPE_SYSTEM_ENCLOSURE)
                    break;

                smbios_patcher_patch_entry_string(entry, entry->data.system_enclosure.Manufacturer_,
                                                  qemu_opt_get(opts, "manufacturer"));
                smbios_patcher_patch_entry_string(entry, entry->data.system_enclosure.Version_,
                                                  qemu_opt_get(opts, "version"));
                smbios_patcher_patch_entry_string(entry, entry->data.system_enclosure.SerialNumber_,
                                                  qemu_opt_get(opts, "serial"));
                smbios_patcher_patch_entry_string(entry, entry->data.system_enclosure.AssetTag_,
                                                  qemu_opt_get(opts, "asset"));
                smbios_patcher_patch_entry_string(entry, entry->data.system_enclosure.SKUNumber_,
                                                  qemu_opt_get(opts, "sku"));
                break;

            case TYPE_PROCESSOR_INFO:
                if (type != TYPE_PROCESSOR_INFO)
                    break;

                smbios_patcher_patch_entry_string(
                    entry, entry->data.processor_info.SocketDesignation_,
                    qemu_opt_get(opts, "sock_pfx"));
                smbios_patcher_patch_entry_string(
                    entry, entry->data.processor_info.ProcessorManufacturer_,
                    qemu_opt_get(opts, "manufacturer"));
                smbios_patcher_patch_entry_string(
                    entry, entry->data.processor_info.ProcessorVersion_,
                    qemu_opt_get(opts, "version"));
                smbios_patcher_patch_entry_string(
                    entry, entry->data.processor_info.SerialNumber_,
                    qemu_opt_get(opts, "serial"));
                smbios_patcher_patch_entry_string(
                    entry, entry->data.processor_info.AssetTagNumber_,
                    qemu_opt_get(opts, "asset"));
                smbios_patcher_patch_entry_string(
                    entry, entry->data.processor_info.PartNumber_,
                    qemu_opt_get(opts, "part"));

                // Processor Family
                uint16_t processor_family = qemu_opt_get_number(opts, "processor-family", 0x1);
                if (processor_family != 0x01) {
                    uint8_t *proc_family_ptr = (uint8_t *) entry->rawdata + PROC_OFF_FAMILY;
                    memcpy(proc_family_ptr, &processor_family, sizeof(uint16_t));
                }
                break;

            case TYPE_MEMORY_DEVICE:
                if (type != TYPE_MEMORY_DEVICE)
                    break;

                smbios_patcher_patch_entry_string(
                    entry, entry->data.memory_device.DeviceLocator_,
                    qemu_opt_get(opts, "loc_pfx"));
                smbios_patcher_patch_entry_string(
                    entry, entry->data.memory_device.BankLocator_,
                    qemu_opt_get(opts, "bank"));
                smbios_patcher_patch_entry_string(
                    entry, entry->data.memory_device.Manufacturer_,
                    qemu_opt_get(opts, "manufacturer"));
                smbios_patcher_patch_entry_string(
                    entry, entry->data.memory_device.SerialNumber_,
                    qemu_opt_get(opts, "serial"));
                smbios_patcher_patch_entry_string(
                    entry, entry->data.memory_device.AssetTagNumber_,
                    qemu_opt_get(opts, "asset"));
                smbios_patcher_patch_entry_string(
                    entry, entry->data.memory_device.PartNumber_,
                    qemu_opt_get(opts, "part"));

                uint16_t speed = qemu_opt_get_number(opts, "speed", 0);
                if (speed != 0) {
                    uint16_t *speed_ptr = (uint16_t *) ((uint8_t *) entry->rawdata + MEM_OFF_SPEED);
                    memcpy(speed_ptr, &speed, sizeof(uint16_t));
                }

                break;
            default: ;
        }
    }

    if (err != SMBERR_END_OF_STREAM) {
        error_setg(errp, "Error while parsing SMBIOS entries");
    }

    smbios_reset(&parser);
}

// Helper function to update a string in the SMBIOS entry
// This modifies the string in-place. For longer strings, it will truncate.
void smbios_patcher_patch_entry_string(const struct Entry *entry, const uint8_t string_index,
                                       const char *new_value) {
    if (string_index == 0 || new_value == NULL || entry == NULL)
        return;

    // Find the start of the string area (relative to buffer start)
    const uint8_t *string_area_start = (const uint8_t *) entry->strings;
    uint8_t *current = (uint8_t *) string_area_start;

    // Navigate to the target string
    for (int i = 1; i < string_index && *current != 0; i++) {
        while (*current != 0)
            current++;
        current++;
    }

    if (*current == 0 && string_index > 1) {
        DPRINTF("String index %d out of bounds for entry type %d\n",
                string_index, entry->type);
        return;
    }

    // Find the end of the current string
    const uint8_t *old_string_end = current;
    while (*old_string_end != 0)
        old_string_end++;

    const size_t old_len = old_string_end - current;
    const size_t new_len = strlen(new_value);

    // Replace the string
    if (new_len <= old_len) {
        // New string fits - just copy and pad with spaces if needed
        memcpy(current, new_value, new_len);
        if (new_len < old_len) {
            // Pad remaining space with spaces (not nulls, to maintain string
            // separation)
            memset(current + new_len, ' ', old_len - new_len);
        }
    } else {
        // New string is longer - truncate to fit
        memcpy(current, new_value, old_len);
        DPRINTF("Truncated string to fit: old_len=%zu, new_len=%zu\n",
                old_len, new_len);
    }
}

void smbios_patcher_parse_uuid(const char *uuid_str, uint8_t *uuid_out, Error **errp) {
    unsigned int parts[5];
    unsigned int bytes[8];

    // Parse UUID format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    // Fourth and fifth groups need to be parsed as individual bytes
    if (sscanf(uuid_str, "%8x-%4x-%4x-%2x%2x-%2x%2x%2x%2x%2x%2x",
               &parts[0], &parts[1], &parts[2],
               &bytes[0], &bytes[1], &bytes[2], &bytes[3],
               &bytes[4], &bytes[5], &bytes[6], &bytes[7]) != 11) {
        error_setg(errp, "Invalid UUID");
        return;
    }

    // First group (32-bit, little-endian)
    uuid_out[0] = (parts[0] >> 24) & 0xFF;
    uuid_out[1] = (parts[0] >> 16) & 0xFF;
    uuid_out[2] = (parts[0] >> 8) & 0xFF;
    uuid_out[3] = parts[0] & 0xFF;

    // Second group (16-bit, little-endian)
    uuid_out[4] = (parts[1] >> 8) & 0xFF;
    uuid_out[5] = parts[1] & 0xFF;

    // Third group (16-bit, little-endian)
    uuid_out[6] = (parts[2] >> 8) & 0xFF;
    uuid_out[7] = parts[2] & 0xFF;

    // Fourth and fifth groups (64-bit, big-endian)
    for (int i = 0; i < 8; i++)
        uuid_out[8 + i] = bytes[i] & 0xFF;
}
