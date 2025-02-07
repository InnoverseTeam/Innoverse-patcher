#ifndef _PATCHER_H
#define _PATCHER_H

typedef struct URL_Patch {
    unsigned int address;
    char url[80];
} URL_Patch;

static const URL_Patch url_patches[] = {
        //nim-boss .rodata
        {0xE2282550, "http://pushmore.wup.shop.nextendo.online/pushmore/r/%s"},
        {0xE229A0A0, "http://npns-dev.c.app.nextendo.online/bst.dat"},
        {0xE229A0D0, "http://npns-dev.c.app.nextendo.online/bst2.dat"},
        {0xE2281964, "https://tagaya.wup.shop.nextendo.online/tagaya/versionlist/%s/%s/%s"},
        {0xE22819B4, "https://tagaya.wup.shop.nextendo.online/tagaya/versionlist/%s/%s/latest_version"},
        {0xE2282584, "http://pushmo.wup.shop.nextendo.online/pushmo/d/%s/%u"},
        {0xE22825B8, "http://pushmo.wup.shop.nextendo.online/pushmo/c/%u/%u"},
        {0xE2282DB4, "https://ecs.wup.shop.nextendo.online/ecs/services/ECommerceSOAP"},
        {0xE22830A0, "https://ecs.wup.shop.nextendo.online/ecs/services/ECommerceSOAP"},
        {0xE22830E0, "https://nus.wup.shop.nextendo.online/nus/services/NetUpdateSOAP"},
        {0xE2299990, "nppl.app.nextendo.online"},
        {0xE229A600, "https://pls.wup.shop.nextendo.online/pls/upload"},
        {0xE229A6AC, "https://npvk-dev.app.nextendo.online/reports"},
        {0xE229A6D8, "https://npvk.app.nextendo.online/reports"},
        {0xE229B1F4, "https://npts.app.nextendo.online/p01/tasksheet/%s/%s/%s/%s?c=%s&l=%s"},
        {0xE229B238, "https://npts.app.nextendo.online/p01/tasksheet/%s/%s/%s?c=%s&l=%s"},
        {0xE22AB2D8, "https://idbe-wup.cdn.nextendo.online/icondata/%02X/%016llX.idbe"},
        {0xE22AB318, "https://idbe-ctr.cdn.nextendo.online/icondata/%02X/%016llX.idbe"},
        {0xE22AB358, "https://idbe-wup.cdn.nextendo.online/icondata/%02X/%016llX-%d.idbe"},
        {0xE22AB398, "https://idbe-ctr.cdn.nextendo.online/icondata/%02X/%016llX-%d.idbe"},
        {0xE22B3EF8, "https://ecs.c.shop.nextendo.online"},
        {0xE22B3F30, "https://ecs.c.shop.nextendo.online/ecs/services/ECommerceSOAP"},
        {0xE22B3F70, "https://ias.c.shop.nextendo.online/ias/services/IdentityAuthenticationSOAP"},
        {0xE22B3FBC, "https://cas.c.shop.nextendo.online/cas/services/CatalogingSOAP"},
        {0xE22B3FFC, "https://nus.c.shop.nextendo.online/nus/services/NetUpdateSOAP"},
        {0xE229DE0C, "n.app.nextendo.online"},
        //nim-boss .bss
        {0xE24B8A24, "https://nppl.app.nextendo.online/p01/policylist/1/1/UNK"},
        {0xE31930D4, "https://%s%saccount.nextendo.online/v%u/api/"}

};

#endif
