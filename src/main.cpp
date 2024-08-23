/*  Copyright 2022 Pretendo Network contributors <pretendo.network>
    Copyright 2022 Ash Logan <ash@heyquark.com>
    Copyright 2019 Maschell

    Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
    granted, provided that the above copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
    INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
    IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
    PERFORMANCE OF THIS SOFTWARE.
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wups.h>
#include <optional>
#include <nsysnet/nssl.h>
#include <sysapp/title.h>
#include <coreinit/cache.h>
#include <coreinit/dynload.h>
#include <coreinit/mcp.h>
#include <coreinit/memory.h>
#include <coreinit/memorymap.h>
#include <coreinit/memexpheap.h>
#include <coreinit/title.h>
#include <notifications/notifications.h>
#include <utils/logger.h>
#include "iosu_url_patches.h"
#include "config.h"
#include "Notification.h"
#include "patches/olv_urls.h"
#include "patches/game_matchmaking.h"
#include "patches/account_settings.h"

#include <coreinit/filesystem.h>
#include <cstring>
#include <string>
#include <nn/erreula/erreula_cpp.h>
#include <nn/act/client_cpp.h>

#include <curl/curl.h>
#include "ca_pem.h"

#include <gx2/surface.h>

#define INNOVERSE_VERSION "v1.0.6"

WUPS_PLUGIN_NAME("Innoverse-patcher");
WUPS_PLUGIN_DESCRIPTION("Innoverse Miiverse Custom Patcher");
WUPS_PLUGIN_VERSION(INNOVERSE_VERSION);
WUPS_PLUGIN_AUTHOR("InnoverseTeam");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_STORAGE("innoverse");

WUPS_USE_WUT_DEVOPTAB();

#include <kernel/kernel.h>
#include <mocha/mocha.h>
#include <function_patcher/function_patching.h>
#include "utils/sysconfig.h"

//thanks @Gary#4139 :p
static void write_string(uint32_t addr, const char *str) {
    int len = strlen(str) + 1;
    int remaining = len % 4;
    int num = len - remaining;

    for (int i = 0; i < (num / 4); i++) {
        Mocha_IOSUKernelWrite32(addr + i * 4, *(uint32_t * )(str + i * 4));
    }

    if (remaining > 0) {
        uint8_t buf[4];
        Mocha_IOSUKernelRead32(addr + num, (uint32_t * ) & buf);

        for (int i = 0; i < remaining; i++) {
            buf[i] = *(str + num + i);
        }

        Mocha_IOSUKernelWrite32(addr + num, *(uint32_t * ) & buf);
    }
}

static bool is555(MCPSystemVersion version) {
    return (version.major == 5) && (version.minor == 5) && (version.patch >= 5);
}

static const char *get_nintendo_network_message() {
    switch (get_system_language()) {
        case nn::swkbd::LanguageType::English:
        default:
            return "Using Miiverse";
        case nn::swkbd::LanguageType::Spanish:
        case nn::swkbd::LanguageType::Portuguese:
        case nn::swkbd::LanguageType::Italian:
            return "Usando Miiverse";
        case nn::swkbd::LanguageType::French:
            return "Sur Miiverse";
        case nn::swkbd::LanguageType::German:
            return "Nutze Miiverse";
        case nn::swkbd::LanguageType::SimplifiedChinese:
        case nn::swkbd::LanguageType::TraditionalChinese:
            return "使用 Miiverse";
        case nn::swkbd::LanguageType::Japanese:
            return "ニンテンドーネットワークを使用中";
        case nn::swkbd::LanguageType::Dutch:
            return "Miiverse wordt gebruikt";
        case nn::swkbd::LanguageType::Russian:
            return "Используется Miiverse";
    }
}

static const char *get_innoverse_message() {
    switch (get_system_language()) {
        case nn::swkbd::LanguageType::English:
        default:
            return "Using Innoverse";
        case nn::swkbd::LanguageType::Spanish:
        case nn::swkbd::LanguageType::Portuguese:
        case nn::swkbd::LanguageType::Italian:
            return "Usando Innoverse";
        case nn::swkbd::LanguageType::French:
            return "Sur Innoverse";
        case nn::swkbd::LanguageType::German:
            return "Nutze Innoverse";
        case nn::swkbd::LanguageType::SimplifiedChinese:
        case nn::swkbd::LanguageType::TraditionalChinese:
            return "使用 Innoverse";
        case nn::swkbd::LanguageType::Japanese:
            return "Innoverseの使用例";
        case nn::swkbd::LanguageType::Dutch:
            return "Innoverse wordt gebruikt";
        case nn::swkbd::LanguageType::Russian:
            return "Используется Innoverse";
    }
}

INITIALIZE_PLUGIN() {
    WHBLogCafeInit();
    WHBLogUdpInit();

    Config::Init();

    auto res = Mocha_InitLibrary();

    if (res != MOCHA_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE("Mocha init failed with code %d!", res);
        return;
    }

    if (NotificationModule_InitLibrary() != NOTIFICATION_MODULE_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE("NotificationModule_InitLibrary failed");
    }

    //get os version
    MCPSystemVersion os_version;
    int mcp = MCP_Open();
    int ret = MCP_GetSystemVersion(mcp, &os_version);
    if (ret < 0) {
        DEBUG_FUNCTION_LINE("getting system version failed (%d/%d)!", mcp, ret);
        os_version = (MCPSystemVersion) {
                .major = 5, .minor = 5, .patch = 5, .region = 'E'
        };
    }
    DEBUG_FUNCTION_LINE_VERBOSE("Running on %d.%d.%d%c",
                                os_version.major, os_version.minor, os_version.patch, os_version.region
    );

    // if using Innoverse then (try to) apply the ssl patches
    if (Config::connect_to_network) {
        if (is555(os_version)) {
            Mocha_IOSUKernelWrite32(0xE1019F78, 0xE3A00001); // mov r0, #1
        } else {
            Mocha_IOSUKernelWrite32(0xE1019E84, 0xE3A00001); // mov r0, #1
        }

        for (const auto &patch: url_patches) {
            write_string(patch.address, patch.url);
        }

        // IOS-NIM-BOSS GlobalPolicyList->state: poking this forces a refresh after we changed the url
        Mocha_IOSUKernelWrite32(0xE24B3D90, 4);
        
        DEBUG_FUNCTION_LINE_VERBOSE("Innoverse URL and NoSSL patches applied successfully.");

        ShowNotification(get_innoverse_message());
    } else {
        DEBUG_FUNCTION_LINE_VERBOSE("Innoverse URL and NoSSL patches skipped.");

        ShowNotification(get_nintendo_network_message());
    }

    MCP_Close(mcp);

    if (FunctionPatcher_InitLibrary() == FUNCTION_PATCHER_RESULT_SUCCESS) {
        install_matchmaking_patches();
    }
}

DEINITIALIZE_PLUGIN() {
    remove_matchmaking_patches();

    Mocha_DeInitLibrary();
    NotificationModule_DeInitLibrary();
    FunctionPatcher_DeInitLibrary();

    WHBLogCafeDeinit();
    WHBLogUdpDeinit();
}

ON_APPLICATION_START() {
    DEBUG_FUNCTION_LINE_VERBOSE("Innoverse " INNOVERSE_VERSION " starting up...\n");

    setup_olv_libs();
    matchmaking_notify_titleswitch();
    patchAccountSettings();
}

ON_APPLICATION_ENDS() {
    DEBUG_FUNCTION_LINE_VERBOSE("Innoverse " INNOVERSE_VERSION " shuting down...\n");
}
