#include "sysconfig.h"
#include "utils/logger.h"

#include <coreinit/userconfig.h>
#include <optional>

nn::swkbd::LanguageType get_system_language() {
    static std::optional <nn::swkbd::LanguageType> cached_language{};
    if (cached_language) return *cached_language;

    UCHandle handle = UCOpen();
    if (handle >= 0) {
        nn::swkbd::LanguageType language;

        UCSysConfig settings __attribute__((__aligned__(0x40))) = {
                .name = "cafe.language",
                .access = 0,
                .dataType = UC_DATATYPE_UNSIGNED_INT,
                .error = UC_ERROR_OK,
                .dataSize = sizeof(language),
                .data = &language,
        };

        UCError err = UCReadSysConfig(handle, 1, &settings);
        UCClose(handle);
        if (err != UC_ERROR_OK) {
            DEBUG_FUNCTION_LINE("Error reading UC: %d!", err);
            return nn::swkbd::LanguageType::English;
        } else {
            DEBUG_FUNCTION_LINE_VERBOSE("System language found: %d", language);
            cached_language = language;
            return language;
        }
    } else {
        DEBUG_FUNCTION_LINE("Error opening UC: %d", handle);
        return nn::swkbd::LanguageType::English;
    }
}
