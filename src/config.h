#ifndef CONFIG_H
#define CONFIG_H

class Config {
public:
    static void Init();

    static bool connect_to_network;

    static bool need_relaunch;
    
    static bool is_wiiu_menu;

    static bool unregister_task_item_pressed;
};

#endif CONFIG_H
