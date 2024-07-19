//
// Created by ash.
//
#include "config.h"

#include "wut_extra.h"
#include "utils/logger.h"
#include "utils/sysconfig.h"

#include <wups.h>
#include <wups/storage.h>
#include <wups/config_api.h>
#include <wups/config/WUPSConfigItemBoolean.h>

#include <coreinit/title.h>
#include <coreinit/launch.h>
#include <sysapp/title.h>
#include <sysapp/launch.h>
#include <nn/act.h>

bool Config::connect_to_network = true;
bool Config::need_relaunch = false;
bool Config::unregister_task_item_pressed = false;
bool Config::is_wiiu_menu = false;

config_strings strings;

constexpr config_strings get_config_strings(nn::swkbd::LanguageType language) {
    switch (language) {
        case nn::swkbd::LanguageType::English:
        default:
            return {
                .plugin_name = "Innoverse-patcher",
                .network_category = "Network selection",
                .connect_to_network_setting = "Connect to the Innoverse",
                .other_category = "Other settings",
                .reset_wwp_setting = "Reset Wara Wara Plaza",
                .press_a_action = "Press A",
                .restart_to_apply_action = "Restart to apply",
                .need_menu_action = "From WiiU menu only",
                .restart_console = "Restart the patches (When you have a problem)",
            };

        case nn::swkbd::LanguageType::Spanish:
            return {
                .plugin_name = "Innoverse-patcher",
                .network_category = "Selección de red",
                .connect_to_network_setting = "Conectar a la red Innoverse",
                .other_category = "Otros ajustes",
                .reset_wwp_setting = "Restablecer Wara Wara Plaza",
                .press_a_action = "Pulsa A",
                .restart_to_apply_action = "Reinicia para confirmar",
                .need_menu_action = "Sólo desde el menú de WiiU",
                .restart_console = "Parche de reinicio (en caso de problemas)",
            };

        case nn::swkbd::LanguageType::French:
            return {
                .plugin_name = "Innoverse-patcher",
                .network_category = "Sélection du réseau",
                .connect_to_network_setting = "Connexion à Innoverse",
                .other_category = "Autres paramètres",
                .reset_wwp_setting = "Réinitialiser la place WaraWara",
                .press_a_action = "Appuyez sur A",
                .restart_to_apply_action = "Redémarrer pour appliquer",
                .need_menu_action = "Depuis le menu Wii U seulement",
                .restart_console = "Redémarrer les patches (Si vous avez un problème)",
            };
        case nn::swkbd::LanguageType::Italian:
            return {
                .plugin_name = "Innoverse-patcher",
                .network_category = "Selezione della rete",
                .connect_to_network_setting = "Connettiti alla rete Innoverse",
                .other_category = "Altre categorie",
                .reset_wwp_setting = "Ripristina Wara Wara Plaza",
                .press_a_action = "Premi A",
                .restart_to_apply_action = "Riavvia per applicare",
                .need_menu_action = "Solo dal menu WiiU",
                .restart_console = "Patch di riavvio (in caso di problemi)",
            };

        case nn::swkbd::LanguageType::German:
            return {
                .plugin_name = "Innoverse-patcher",
                .network_category = "Netzwerkauswahl",
                .connect_to_network_setting = "Verbinde zum Innoverse",
                .other_category = "Andere Einstellungen",
                .reset_wwp_setting = "Wara Wara Plaza zurücksetzen",
                .press_a_action = "Drücke A",
                .restart_to_apply_action = "Neustarten zum Anwenden",
                .need_menu_action = "Nur vom Wii U-Menü aus",
                .restart_console = "Neustart-Patch (bei Problemen)",
            };
    }
}

static void connect_to_network_changed(ConfigItemBoolean* item, bool new_value) {
    DEBUG_FUNCTION_LINE("connect_to_network changed to: %d", new_value);
    if (new_value != Config::connect_to_network) {
        Config::need_relaunch = true;
    }
    Config::connect_to_network = new_value;
    if (WUPSStorageAPI::Store<bool>("connect_to_network", Config::connect_to_network) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE("Failed to save \"connect_to_network\" value (%d)", Config::connect_to_network);
    }
}

static void unregister_task_item_on_input_cb(void *context, WUPSConfigSimplePadData input) {
    if (!Config::unregister_task_item_pressed && Config::is_wiiu_menu && ((input.buttons_d & WUPS_CONFIG_BUTTON_A) == WUPS_CONFIG_BUTTON_A)) {

        nn::act::Initialize();
        Initialize__Q2_2nn4bossFv();

        for (uint8_t i = 1; i <= nn::act::GetNumOfAccounts(); i++)
        {
            if (nn::act::IsSlotOccupied(i) && nn::act::IsNetworkAccountEx(i))
            {
              nn::boss::Task task{};
              nn::act::PersistentId persistentId = nn::act::GetPersistentIdEx(i);

              __ct__Q3_2nn4boss4TaskFv(&task);
              Initialize__Q3_2nn4boss4TaskFPCcUi(&task, "oltopic", persistentId);

      // bypasses compiler warning about unused variable
      #ifdef DEBUG
              uint32_t res = Unregister__Q3_2nn4boss4TaskFv(&task);
              DEBUG_FUNCTION_LINE_VERBOSE("Unregistered oltopic for: SlotNo %d | Persistent ID %08x -> 0x%08x", i, persistentId, res);
      #else
              Unregister__Q3_2nn4boss4TaskFv(&task);
      #endif
          }
      }

        Finalize__Q2_2nn4bossFv();
        nn::act::Finalize();

        Config::unregister_task_item_pressed = !Config::unregister_task_item_pressed;
        Config::need_relaunch = true;
    }
}

static int32_t unregister_task_item_get_display_value(void *context, char *out_buf, int32_t out_size) {
    if (!Config::is_wiiu_menu)
        strncpy(out_buf, strings.need_menu_action, out_size);
    else
        strncpy(out_buf, Config::unregister_task_item_pressed ? strings.restart_to_apply_action : strings.press_a_action, out_size);

    return 0;
}

static WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
    uint64_t current_title_id = OSGetTitleID();
    uint64_t wiiu_menu_tid = _SYSGetSystemApplicationTitleId(SYSTEM_APP_ID_WII_U_MENU);
    Config::is_wiiu_menu = (current_title_id == wiiu_menu_tid);

    // get translation strings
    strings = get_config_strings(get_system_language());

    // create root config category
    WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

    auto patching_cat = WUPSConfigCategory::Create(strings.network_category);

    //                                                  config id                   display name            default        current value             changed callback
    patching_cat.add(WUPSConfigItemBoolean::Create("connect_to_network", strings.connect_to_network_setting, true, Config::connect_to_network, &connect_to_network_changed));
    root.add(std::move(patching_cat));

    auto other_cat = WUPSConfigCategory::Create(strings.other_category);

    WUPSConfigAPIItemCallbacksV2 unregisterTasksItemCallbacks = {
            .getCurrentValueDisplay = unregister_task_item_get_display_value,
            .getCurrentValueSelectedDisplay = unregister_task_item_get_display_value,
            .onSelected = nullptr,
            .restoreDefault = nullptr,
            .isMovementAllowed = nullptr,
            .onCloseCallback = nullptr,
            .onInput = unregister_task_item_on_input_cb,
            .onInputEx = nullptr,
            .onDelete = nullptr
    };

    WUPSConfigAPIItemOptionsV2 unregisterTasksItemOptions = {
            .displayName = strings.reset_wwp_setting,
            .context = nullptr,
            .callbacks = unregisterTasksItemCallbacks,
    };

    WUPSConfigItemHandle unregisterTasksItem;
    WUPSConfigAPIStatus err;
    if ((err = WUPSConfigAPI_Item_Create(unregisterTasksItemOptions, &unregisterTasksItem)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE("Creating config menu failed: %s", WUPSConfigAPI_GetStatusStr(err));
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
    if ((err = WUPSConfigAPI_Category_AddItem(other_cat.getHandle(), unregisterTasksItem)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE("Creating config menu failed: %s", WUPSConfigAPI_GetStatusStr(err));
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }

    root.add(std::move(other_cat));

    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

static void ConfigMenuClosedCallback() {
    // Save all changes
    if (WUPSStorageAPI::SaveStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE("Failed to save storage");
    }

    if (Config::need_relaunch) {
        // Need to reload the console so the patches reset
        OSForceFullRelaunch();
        SYSLaunchMenu();
        Config::need_relaunch = false;
    }
}

void Config::Init() {
    // Init the config api
    WUPSConfigAPIOptionsV1 configOptions = { .name = "Innoverse-patcher" };
    if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE("Failed to initialize WUPS Config API");
        return;
    }

    WUPSStorageError storageRes;
    // Try to get value from storage
    if ((storageRes = WUPSStorageAPI::Get<bool>("connect_to_network", Config::connect_to_network)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
        DEBUG_FUNCTION_LINE("Connect to network value not found, attempting to migrate/create");

        bool skipPatches = false;
        if (WUPSStorageAPI::Get<bool>("skipPatches", skipPatches) == WUPS_STORAGE_ERROR_SUCCESS) {
            // Migrate old config value
            Config::connect_to_network = !skipPatches;
            WUPSStorageAPI::DeleteItem("skipPatches");
        }
    
        // Add the value to the storage if it's missing.
        if (WUPSStorageAPI::Store<bool>("connect_to_network", connect_to_network) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE("Failed to store bool");
        }
    }
    else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE("Failed to get bool %s (%d)", WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
    }

    // Save storage
    if (WUPSStorageAPI::SaveStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE("Failed to save storage");
    }
}
