#include "Notification.h"
#include <notifications/notification_defines.h>
#include <notifications/notifications.h>

void ShowNotification(std::string_view notification) {
    auto err1 = NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO,
                                                   NOTIFICATION_MODULE_DEFAULT_OPTION_KEEP_UNTIL_SHOWN, true);
    auto err2 = NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO,
                                                   NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT,
                                                   15.0f);

    if (err1 != NOTIFICATION_MODULE_RESULT_SUCCESS || err2 != NOTIFICATION_MODULE_RESULT_SUCCESS) return;

    NotificationModule_AddInfoNotification(notification.data());
}
