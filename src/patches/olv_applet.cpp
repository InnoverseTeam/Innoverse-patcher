/*  Copyright 2023 Pretendo Network contributors <pretendo.network>
    Copyright 2023 Ash Logan <ash@heyquark.com>
    Copyright 2019 Maschell

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

const char wave_original[] = {
        0x68, 0x74, 0x74, 0x70, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x2E, 0x6E, 0x69, 0x6E, 0x74, 0x65, 0x6E, 0x64,
        0x6F, 0x2E, 0x6E, 0x65, 0x74
};
const char wave_new[] = {
        0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x6E, 0x70, 0x70, 0x6C, 0x2D, 
        0x69, 0x6E, 0x6E, 0x6F, 0x76, 0x65, 0x72, 0x73, 0x65, 0x2D, 0x61, 0x70, 
        0x70, 0x2E, 0x61, 0x6C, 0x77, 0x61, 0x79, 0x73, 0x64, 0x61, 0x74, 0x61, 
        0x2E, 0x6E, 0x65, 0x74
};
const unsigned char miiverse_green_highlight[] = {
        0x82, 0xff, 0x05, 0xff, 0x82, 0xff, 0x05, 0xff, 0x1d, 0xff, 0x04, 0xff, 0x1d, 0xff, 0x04, 0xff
};
const unsigned char inno_blue_highlight[] = {
        0x05, 0x82, 0xff, 0xff, 0x05, 0x82, 0xff, 0xff, 0x1d, 0x04, 0xff, 0xff, 0x1d, 0x04, 0xff, 0xff
};
const unsigned char miiverse_green_touch1[] = {
        0x94, 0xd9, 0x2a, 0x00, 0x57, 0xbd, 0x12, 0xff
};
const unsigned char inno_blue_touch1[] = {
        0x2a, 0x94, 0xd9, 0x00, 0x12, 0x57, 0xbd, 0xff
};
const unsigned char miiverse_green_touch2[] = {
        0x57, 0xbd, 0x12, 0x00, 0x94, 0xd9, 0x2a, 0xff
};
const unsigned char inno_blue_touch2[] = {
        0x12, 0x57, 0xbd, 0x00, 0x2a, 0x94, 0xd9, 0xff
};

const replacement replacements[] = {
        {miiverse_green_highlight, inno_blue_highlight},
        {miiverse_green_touch1,    inno_blue_touch1},
        {miiverse_green_touch2,    inno_blue_touch2},
};

static std::optional<FSFileHandle> rootca_pem_handle{};

DECL_FUNCTION(int, FSOpenFile, FSClient *client, FSCmdBlock *block, char *path, const char *mode, uint32_t *handle,
              int error) {
    const char *initialOma = "vol/content/initial.oma";

    if (!Config::connect_to_network) {
        DEBUG_FUNCTION_LINE("Innoverse-patcher: Miiverse patches skipped.");
        return real_FSOpenFile(client, block, path, mode, handle, error);
    }

    if (strcmp(initialOma, path) == 0) {
        //below is a hacky (yet functional!) way to get Innoverse-patcher to redirect URLs from the Miiverse applet
        //we do it when loading this file since it should only load once, preventing massive lag spikes as it searches all of MEM2 xD
        //WHBLogUdpInit();

        DEBUG_FUNCTION_LINE("Innoverse-patcher: hellooooo!\n");

        auto olv_ok = setup_olv_libs();
        // Patch applet binary too
        if (olv_ok)
            replace(0x10000000, 0x10000000, wave_original, sizeof(wave_original), wave_new, sizeof(wave_new));
        // Check for root CA file and take note of its handle
    } else if (strcmp("vol/content/browser/rootca.pem", path) == 0) {
        int ret = real_FSOpenFile(client, block, path, mode, handle, error);
        rootca_pem_handle = *handle;
        DEBUG_FUNCTION_LINE("Innoverse-patcher: Found Miiverse CA, replacing...");
        return ret;
    }

    return real_FSOpenFile(client, block, path, mode, handle, error);
}

DECL_FUNCTION(FSStatus, FSReadFile, FSClient *client, FSCmdBlock *block, uint8_t *buffer, uint32_t size, uint32_t count,
              FSFileHandle handle, uint32_t unk1, uint32_t flags) {
    if (size != 1) {
        DEBUG_FUNCTION_LINE("Innoverse-patcher: Miiverse CA replacement failed!");
    }

    if (rootca_pem_handle && *rootca_pem_handle == handle) {
        strlcpy((char *) buffer, (const char *) ca_pem, size * count);

        //this can't be done above (in the FSOpenFile hook) since it's not loaded yet.
        //the hardcoded offsets suck but they really are at Random Places In The Heap
        replaceBulk(0x11000000, 0x02000000, replacements);
        return (FSStatus) count;
    }

    return real_FSReadFile(client, block, buffer, size, count, handle, unk1, flags);
}

DECL_FUNCTION(FSStatus, FSCloseFile, FSClient *client, FSCmdBlock *block, FSFileHandle handle, FSErrorFlag errorMask) {
    if (handle == rootca_pem_handle) {
        rootca_pem_handle.reset();
    }

    return real_FSCloseFile(client, block, handle, errorMask);
}

DECL_FUNCTION(uint32_t, NSSLExportInternalServerCertificate, NSSLServerCertId cert, int unk, void *unk2, void *unk3) {
    if (cert == NSSL_SERVER_CERT_THAWTE_PREMIUM_SERVER_CA) { // Martini patches
        OSFatal("[598-0069] Please uninstall Martini patches to continue.\n" \
                "See pretendo.network/docs/search for more info.\n\n" \
                "Hold the POWER button for 4 seconds to shut down.\n\n"
                "            .\n"
                ".---------.'---.\n"
                "'.       :    .'\n"
                "  '.  .:::  .'\n"
                "    '.'::'.'\n"
                "      '||'\n"
                "       ||\n"
                "       ||\n"
                "mrz    ||\n"
                "   ---====---");
    }
    return real_NSSLExportInternalServerCertificate(cert, unk, unk2, unk3);
}

WUPS_MUST_REPLACE_FOR_PROCESS(FSOpenFile, WUPS_LOADER_LIBRARY_COREINIT, FSOpenFile, WUPS_FP_TARGET_PROCESS_MIIVERSE);
WUPS_MUST_REPLACE_FOR_PROCESS(FSReadFile, WUPS_LOADER_LIBRARY_COREINIT, FSReadFile, WUPS_FP_TARGET_PROCESS_MIIVERSE);
WUPS_MUST_REPLACE_FOR_PROCESS(FSCloseFile, WUPS_LOADER_LIBRARY_COREINIT, FSCloseFile, WUPS_FP_TARGET_PROCESS_MIIVERSE);
WUPS_MUST_REPLACE_FOR_PROCESS(NSSLExportInternalServerCertificate, WUPS_LOADER_LIBRARY_NSYSNET,
                              NSSLExportInternalServerCertificate, WUPS_FP_TARGET_PROCESS_MIIVERSE);
