#include "config.h"
#include "olv_urls.h"
#include "utils/logger.h"
#include "utils/replace_mem.h"

#include <wups.h>
#include <optional>
#include <coreinit/debug.h>
#include <coreinit/filesystem.h>
#include <nsysnet/nssl.h>

#include "ca_pem.h" // generated at buildtime

const char wave_original[] = "https://ninja.wup.shop.nintendo.net/ninja/wood_index.html?";
const char wave_new[] =      "http://geteshop-cdn.alwaysdata.net/ninja/wood_index.html?";

const char whitelist_original[] = {
        0x68, 0x74, 0x74, 0x70, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x73, 0x61, 0x6D, 0x75, 0x72, 0x61, 0x69, 0x2E,
        0x77, 0x75, 0x70, 0x2E, 0x73, 0x68, 0x6F, 0x70, 0x2E, 0x6E, 0x69, 0x6E,
        0x74, 0x65, 0x6E, 0x64, 0x6F, 0x2E, 0x6E, 0x65, 0x74
};

const char whitelist_new[] = {
        0x68, 0x74, 0x74, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x73, 0x61, 0x6D, 0x75, 0x72, 0x61, 0x69, 0x2E,
        0x77, 0x75, 0x70, 0x2E, 0x73, 0x68, 0x6F, 0x70, 0x2E, 0x70, 0x72, 0x65,
        0x74, 0x65, 0x6E, 0x64, 0x6F, 0x2E, 0x63, 0x63, 0x00
};

static std::optional<FSFileHandle> rootca_pem_handle{};

DECL_FUNCTION(int, FSOpenFile_eShop, FSClient *client, FSCmdBlock *block, char *path, const char *mode, uint32_t *handle,
              int error) {
    const char *initialOma = "vol/content/initial.oma";

    if (!Config::connect_to_network) {
        DEBUG_FUNCTION_LINE("Innoverse-patcher: eShop patches skipped.");
        return real_FSOpenFile_eShop(client, block, path, mode, handle, error);
    }

    if (strcmp(initialOma, path) == 0) {
        DEBUG_FUNCTION_LINE("Innoverse-patcher: hewwo eShop!\n");

        if (!replace(0x10000000, 0x10000000, wave_original, sizeof(wave_original), wave_new, sizeof(wave_new)))
            DEBUG_FUNCTION_LINE("Innoverse-patcher: We didn't find the url /)>~<(\\");

        if (!replace(0x10000000, 0x10000000, whitelist_original, sizeof(whitelist_original), whitelist_new, sizeof(whitelist_new)))
            DEBUG_FUNCTION_LINE("Innoverse-patcher: We didn't find the whitelist /)>~<(\\");

    // Check for root CA file and take note of its handle
    } else if (strcmp("vol/content/browser/rootca.pem", path) == 0) {
        int ret = real_FSOpenFile_eShop(client, block, path, mode, handle, error);
        rootca_pem_handle = *handle;
        DEBUG_FUNCTION_LINE("Innoverse-patcher: Found eShop CA, replacing...");
        return ret;
    }

    return real_FSOpenFile_eShop(client, block, path, mode, handle, error);
}

DECL_FUNCTION(FSStatus, FSReadFile_eShop, FSClient *client, FSCmdBlock *block, uint8_t *buffer, uint32_t size, uint32_t count,
              FSFileHandle handle, uint32_t unk1, uint32_t flags) {
    if (size != 1) {
        DEBUG_FUNCTION_LINE("Innoverse-patcher: eShop CA replacement failed!");
    }

    if (rootca_pem_handle && *rootca_pem_handle == handle) {
        strlcpy((char *) buffer, (const char *) ca_pem, size * count);
        return (FSStatus) count;
    }

    return real_FSReadFile_eShop(client, block, buffer, size, count, handle, unk1, flags);
}

DECL_FUNCTION(FSStatus, FSCloseFile_eShop, FSClient *client, FSCmdBlock *block, FSFileHandle handle, FSErrorFlag errorMask) {
    if (handle == rootca_pem_handle) {
        rootca_pem_handle.reset();
    }

    return real_FSCloseFile_eShop(client, block, handle, errorMask);
}

WUPS_MUST_REPLACE_FOR_PROCESS(FSOpenFile_eShop, WUPS_LOADER_LIBRARY_COREINIT, FSOpenFile, WUPS_FP_TARGET_PROCESS_ESHOP);
WUPS_MUST_REPLACE_FOR_PROCESS(FSReadFile_eShop, WUPS_LOADER_LIBRARY_COREINIT, FSReadFile, WUPS_FP_TARGET_PROCESS_ESHOP);
WUPS_MUST_REPLACE_FOR_PROCESS(FSCloseFile_eShop, WUPS_LOADER_LIBRARY_COREINIT, FSCloseFile, WUPS_FP_TARGET_PROCESS_ESHOP);
