#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wups.h>
#include <optional>
#include <nsysnet/nssl.h>
#include <coreinit/cache.h>
#include <coreinit/dynload.h>
#include <coreinit/mcp.h>
#include <coreinit/memory.h>
#include <coreinit/memorymap.h>
#include <coreinit/memexpheap.h>
#include <notifications/notifications.h>
#include <utils/logger.h>
#include "iosu_url_patches.h"
#include "config.h"
#include "Notification.h"
#include "patches/olv_urls.h"
#include "patches/game_matchmaking.h"

#include <coreinit/filesystem.h>
#include <cstring>
#include <string>
#include <nn/erreula/erreula_cpp.h>
#include <nn/act/client_cpp.h>

#include <curl/curl.h>
#include "ca_pem.h"

#include <gx2/surface.h>

WUPS_PLUGIN_NAME("Innoverse-Patcher");
WUPS_PLUGIN_DESCRIPTION("Innoverse Patcher");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Innoverse-patcher made by cedkeChat01");
WUPS_PLUGIN_LICENSE("ISC");

WUPS_USE_STORAGE("innoverse-patcher");
WUPS_USE_WUT_DEVOPTAB();

#include <kernel/kernel.h>
#include <mocha/mocha.h>
#include <function_patcher/function_patching.h>

static void write_string(uint32_t addr, const char* str)
{
    int len = strlen(str) + 1;
    int remaining = len % 4;
    int num = len - remaining;

    for (int i = 0; i < (num / 4); i++) {
        Mocha_IOSUKernelWrite32(addr + i * 4, *(uint32_t*)(str + i * 4));
    }

    if (remaining > 0) {
        uint8_t buf[4];
        Mocha_IOSUKernelRead32(addr + num, (uint32_t*)&buf);

        for (int i = 0; i < remaining; i++) {
            buf[i] = *(str + num + i);
        }

        Mocha_IOSUKernelWrite32(addr + num, *(uint32_t*)&buf);
    }
}

static bool is555(MCPSystemVersion version) {
    return (version.major == 5) && (version.minor == 5) && (version.patch >= 5);
}

static const char *get_nintendo_network_message() {
    switch (get_system_language()) {
        case nn::swkbd::LanguageType::English:
        default:
            return "Using Nintendo Network";
        case nn::swkbd::LanguageType::Spanish:
            return "Usando Nintendo Network";
        case nn::swkbd::LanguageType::French:
            return "Sur Nintendo Network";
        case nn::swkbd::LanguageType::Italian:
            return "Usando Nintendo Network";
        case nn::swkbd::LanguageType::German:
            return "Nutze Nintendo Network";
    }
}

static const char *get_innoverse_message() {
    switch (get_system_language()) {
        case nn::swkbd::LanguageType::English:
        default:
            return "Using Innoverse";
        case nn::swkbd::LanguageType::Spanish:
            return "Usando Innoverse";
        case nn::swkbd::LanguageType::French:
            return "Utilise Innoverse";
        case nn::swkbd::LanguageType::Italian:
            return "Usando Innoverse";
        case nn::swkbd::LanguageType::German:
            return "Nutze Innoverse";
    }
}

INITIALIZE_PLUGIN() {
    WHBLogUdpInit();
    WHBLogCafeInit();

    Config::Init();

    auto res = Mocha_InitLibrary();

    if (res != MOCHA_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE("Mocha init failed with code %d!", res);
        return;
    }

    if (NotificationModule_InitLibrary() != NOTIFICATION_MODULE_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE("NotificationModule_InitLibrary failed");
    }

    MCPSystemVersion os_version;
    int mcp = MCP_Open();
    int ret = MCP_GetSystemVersion(mcp, &os_version);
    if (ret < 0) {
        DEBUG_FUNCTION_LINE("getting system version failed (%d/%d)!", mcp, ret);
        os_version = (MCPSystemVersion) {
                .major = 5, .minor = 5, .patch = 5, .region = 'E'
        };
    }
    DEBUG_FUNCTION_LINE("Running on %d.%d.%d%c",
        os_version.major, os_version.minor, os_version.patch, os_version.region
    );

    if (Config::connect_to_network) {
        if (is555(os_version)) {
            Mocha_IOSUKernelWrite32(0xE1019F78, 0xE3A00001); // mov r0, #1
        }
        else {
            Mocha_IOSUKernelWrite32(0xE1019E84, 0xE3A00001); // mov r0, #1
        }

        for (const auto& patch : url_patches) {
            write_string(patch.address, patch.url);
        }
        DEBUG_FUNCTION_LINE("Innoverse URL and NoSSL patches applied successfully.");
        ShowNotification(get_innoverse_message());
    }
    else {
        DEBUG_FUNCTION_LINE("Innoverse URL and NoSSL patches skipped.");
        ShowNotification(get_nintendo_network_message());
    }

    MCP_Close(mcp);

    if (FunctionPatcher_InitLibrary() == FUNCTION_PATCHER_RESULT_SUCCESS) {
        install_matchmaking_patches();
    }
}
DEINITIALIZE_PLUGIN() {
    remove_matchmaking_patches();

    WHBLogUdpDeinit();
    Mocha_DeInitLibrary();
    NotificationModule_DeInitLibrary();
    FunctionPatcher_DeInitLibrary();
}

ON_APPLICATION_START() {
    WHBLogUdpInit();
    WHBLogCafeInit();

    DEBUG_FUNCTION_LINE("Innoverse-patches == starting up\n");

    setup_olv_libs();
    matchmaking_notify_titleswitch();
}

ON_APPLICATION_ENDS() {
    DEBUG_FUNCTION_LINE("Innoverse-patches == shutting down the service...\n");
    StopNotificationThread();
}
