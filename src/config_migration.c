// ---------------------------------------------------------------------------
// Legacy migration
//
// Called when libconfig fails to parse the file.. the file is in the old
// key=value format.. uses the existing config.c to read all
// values into globals, then wOPLSave() immediately rewrites them in
// the new libconfig format.. phase out eventually
// ---------------------------------------------------------------------------

#include "include/common.h"
#include "include/config.h"
#include "include/ioman.h"
#include "include/util.h"
#include "include/gui.h"
#include "include/renderman.h"
#include "include/system.h"
#include "include/themes.h"
#include "include/lang.h"
#include "include/pad.h"
#include "include/sound.h"
#include "include/lwnbd.h"
#include "include/supportbase.h"
#include "include/config_wopl.h"
#include "include/config_migration.h"
#include "include/tar.h"

#include <libconfig.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef __DEBUG
#include "include/debug.h"
#endif

#ifdef GSM
#include "include/pggsm.h"
#endif

#ifdef CHEAT
#include "include/cheatman.h"
#endif

int cfgMigrateLegacyOPL(const char *path, int *out_theme_id, int *out_lang_id)
{
    config_set_t legacy;
    config_set_t *cfg = configAlloc(CONFIG_OPL, &legacy, (char *)path);
    if (!cfg)
        return 0;

    if (!configRead(cfg)) {
        configClear(cfg);
        return 0;
    }

    const char *temp;
    int value;

    configGetInt(cfg, CONFIG_OPL_SCROLLING, &gScrollSpeed);
    configGetColor(cfg, CONFIG_OPL_BGCOLOR, gDefaultBgColor);
    configGetColor(cfg, CONFIG_OPL_TEXTCOLOR, gDefaultTextColor);
    configGetColor(cfg, CONFIG_OPL_UI_TEXTCOLOR, gDefaultUITextColor);
    configGetColor(cfg, CONFIG_OPL_SEL_TEXTCOLOR, gDefaultSelTextColor);
    configGetColor(cfg, CONFIG_OPL_PLAS_BLEND_COLOR, gDefaultPlasmaBlendColor);
    configGetInt(cfg, CONFIG_OPL_ENABLE_NOTIFICATIONS, &gEnableNotifications);
    configGetInt(cfg, CONFIG_OPL_ENABLE_DISCART, &gDiscEnableArt);
    configGetInt(cfg, CONFIG_OPL_WIDESCREEN, &gWideScreen);
    configGetInt(cfg, CONFIG_OPL_VMODE, &gVMode);
    configGetInt(cfg, CONFIG_OPL_XOFF, &gXOff);
    configGetInt(cfg, CONFIG_OPL_YOFF, &gYOff);
    configGetInt(cfg, CONFIG_OPL_OVERSCAN, &gOverscan);
    configGetInt(cfg, CONFIG_OPL_BDM_CACHE, &bdmCacheSize);
    configGetInt(cfg, CONFIG_OPL_HDD_CACHE, &hddCacheSize);
    configGetInt(cfg, CONFIG_OPL_SMB_CACHE, &smbCacheSize);

    if (configGetStr(cfg, CONFIG_OPL_THEME, &temp))
        *out_theme_id = thmFindGuiID(temp);
    if (configGetStr(cfg, CONFIG_OPL_LANGUAGE, &temp))
        *out_lang_id = lngFindGuiID(temp);

    if (configGetInt(cfg, CONFIG_OPL_SWAP_SEL_BUTTON, &value))
        gSelectButton = value == 0 ? KEY_CIRCLE : KEY_CROSS;

    configGetInt(cfg, CONFIG_OPL_XSENSITIVITY, &gXSensitivity);
    configGetInt(cfg, CONFIG_OPL_YSENSITIVITY, &gYSensitivity);
    configGetInt(cfg, CONFIG_OPL_DISABLE_DEBUG, &gEnableDebug);
    configGetInt(cfg, CONFIG_OPL_BDM_DEBUG, &gBDMDebug);
    configGetInt(cfg, CONFIG_OPL_PS2LOGO, &gPS2Logo);
    configGetInt(cfg, CONFIG_OPL_HDD_GAME_LIST_CACHE, &gHDDGameListCache);
    configGetStrCopy(cfg, CONFIG_OPL_EXIT_PATH, gExitPath, sizeof(gExitPath));
    configGetInt(cfg, CONFIG_OPL_AUTO_SORT, &gAutosort);
    configGetInt(cfg, CONFIG_OPL_AUTO_REFRESH, &gAutoRefresh);
    configGetInt(cfg, CONFIG_OPL_DEFAULT_DEVICE, &gDefaultDevice);
    configGetInt(cfg, CONFIG_OPL_ENABLE_WRITE, &gEnableWrite);
    configGetInt(cfg, CONFIG_OPL_HDD_SPINDOWN, &gHDDSpindown);
    configGetStrCopy(cfg, CONFIG_OPL_MMCE_PREFIX, gMMCEPrefix, sizeof(gMMCEPrefix));
    configGetStrCopy(cfg, CONFIG_OPL_BDM_PREFIX, gBDMPrefix, sizeof(gBDMPrefix));
    configGetStrCopy(cfg, CONFIG_OPL_ETH_PREFIX, gETHPrefix, sizeof(gETHPrefix));
    configGetInt(cfg, CONFIG_OPL_REMEMBER_LAST, &gRememberLastPlayed);
    configGetInt(cfg, CONFIG_OPL_AUTOSTART_LAST, &gAutoStartLastPlayed);
    configGetInt(cfg, CONFIG_OPL_BDM_MODE, &gBDMStartMode);
    configGetInt(cfg, CONFIG_OPL_HDD_MODE, &gHDDStartMode);
    configGetInt(cfg, CONFIG_OPL_ETH_MODE, &gETHStartMode);
    configGetInt(cfg, CONFIG_OPL_APP_MODE, &gAPPStartMode);
    configGetInt(cfg, CONFIG_OPL_FAV_MODE, &gFAVStartMode);
    configGetInt(cfg, CONFIG_OPL_MMCE_MODE, &gMMCEStartMode);
    configGetInt(cfg, CONFIG_OPL_MMCE_SLOT, &gMMCESlot);
    configGetInt(cfg, CONFIG_OPL_MMCEIGR_SLOT, &gMMCEIGRSlot);
    configGetInt(cfg, CONFIG_OPL_MMCE_WAIT_CYCLES, &gMMCEAckWaitCycles);
    configGetInt(cfg, CONFIG_OPL_MMCE_USE_ALARMS, &gMMCEUseAlarms);
    configGetInt(cfg, CONFIG_OPL_ENABLE_USB, &gEnableUSB);
    configGetInt(cfg, CONFIG_OPL_ENABLE_ILINK, &gEnableILK);
    configGetInt(cfg, CONFIG_OPL_ENABLE_MX4SIO, &gEnableMX4SIO);
    configGetInt(cfg, CONFIG_OPL_ENABLE_BDMHDD, &gEnableBdmHDD);
    configGetInt(cfg, CONFIG_OPL_SFX, &gEnableSFX);
    configGetInt(cfg, CONFIG_OPL_BOOT_SND, &gEnableBootSND);
    configGetInt(cfg, CONFIG_OPL_BGM, &gEnableBGM);
    configGetInt(cfg, CONFIG_OPL_SFX_VOLUME, &gSFXVolume);
    configGetInt(cfg, CONFIG_OPL_BOOT_SND_VOLUME, &gBootSndVolume);
    configGetInt(cfg, CONFIG_OPL_BGM_VOLUME, &gBGMVolume);
    configGetStrCopy(cfg, CONFIG_OPL_DEFAULT_BGM_PATH, gDefaultBGMPath, sizeof(gDefaultBGMPath));
#ifdef __DEBUG
    configGetInt(cfg, CONFIG_OPL_MMCE_GAMEID, &gMMCEEnableGameID);
#endif
    configGetInt(cfg, CONFIG_OPL_COVERFLOW_COUNT, &gCoverflowCount);
    if (gCoverflowCount != 3 && gCoverflowCount != 5)
        gCoverflowCount = 3;

    configGetInt(cfg, CONFIG_OPL_COVERFLOW_SCALE, &gCoverflowCenterScale);
    configGetInt(cfg, CONFIG_OPL_COVERFLOW_ANIM, &gCoverflowAnimSpeed);
    configGetInt(cfg, CONFIG_OPL_COVERFLOW_DIM, &gCoverflowDimCovers);

    configClear(cfg);
    return 1;
}

// legacy migration.. phase out eventually
int cfgMigrateLegacyNet(const char *path)
{
    config_set_t legacy;
    config_set_t *cfg = configAlloc(CONFIG_NETWORK, &legacy, (char *)path);
    if (!cfg)
        return 0;

    if (!configRead(cfg)) {
        configClear(cfg);
        return 0;
    }

    const char *temp;

    configGetInt(cfg, CONFIG_NET_ETH_LINKM, &gETHOpMode);
    configGetInt(cfg, CONFIG_NET_PS2_DHCP, &ps2_ip_use_dhcp);
    configGetInt(cfg, CONFIG_NET_SMB_NBNS, &gPCShareAddressIsNetBIOS);
    configGetStrCopy(cfg, CONFIG_NET_SMB_NB_ADDR, gPCShareNBAddress, sizeof(gPCShareNBAddress));
    configGetInt(cfg, CONFIG_NET_SMB_PORT, &gPCPort);
    configGetStrCopy(cfg, CONFIG_NET_SMB_SHARE, gPCShareName, sizeof(gPCShareName));
    configGetStrCopy(cfg, CONFIG_NET_SMB_USER, gPCUserName, sizeof(gPCUserName));
    configGetStrCopy(cfg, CONFIG_NET_SMB_PASSW, gPCPassword, sizeof(gPCPassword));
    configGetStrCopy(cfg, CONFIG_NET_NBD_DEFAULT_EXPORT, gExportName, sizeof(gExportName));

    if (configGetStr(cfg, CONFIG_NET_SMB_IP_ADDR, &temp))
        sscanf(temp, "%d.%d.%d.%d", &pc_ip[0], &pc_ip[1], &pc_ip[2], &pc_ip[3]);
    if (configGetStr(cfg, CONFIG_NET_PS2_IP, &temp))
        sscanf(temp, "%d.%d.%d.%d", &ps2_ip[0], &ps2_ip[1], &ps2_ip[2], &ps2_ip[3]);
    if (configGetStr(cfg, CONFIG_NET_PS2_NETM, &temp))
        sscanf(temp, "%d.%d.%d.%d", &ps2_netmask[0], &ps2_netmask[1], &ps2_netmask[2], &ps2_netmask[3]);
    if (configGetStr(cfg, CONFIG_NET_PS2_GATEW, &temp))
        sscanf(temp, "%d.%d.%d.%d", &ps2_gateway[0], &ps2_gateway[1], &ps2_gateway[2], &ps2_gateway[3]);
    if (configGetStr(cfg, CONFIG_NET_PS2_DNS, &temp))
        sscanf(temp, "%d.%d.%d.%d", &ps2_dns[0], &ps2_dns[1], &ps2_dns[2], &ps2_dns[3]);

    configClear(cfg);
    return 1;
}

int cfgMigrateLegacyGlobalGame(const char *path)
{
    config_set_t tmp;
    config_set_t *old = configAlloc(CONFIG_GAME, &tmp, (char *)path);
    if (!old || !configRead(old)) {
        if (old)
            configClear(old);
        return 0;
    }
#ifdef GSM
    configGetInt(old, CONFIG_ITEM_ENABLEGSM, &gGlobalGameCfg.gsm_enable);
    configGetInt(old, CONFIG_ITEM_GSMVMODE, &gGlobalGameCfg.gsm_vmode);
    configGetInt(old, CONFIG_ITEM_GSMXOFFSET, &gGlobalGameCfg.gsm_xoffset);
    configGetInt(old, CONFIG_ITEM_GSMYOFFSET, &gGlobalGameCfg.gsm_yoffset);
    configGetInt(old, CONFIG_ITEM_GSMFIELDFIX, &gGlobalGameCfg.gsm_fieldfix);
#endif
#ifdef CHEAT
    configGetInt(old, CONFIG_ITEM_ENABLECHEAT, &gGlobalGameCfg.cheat_enable);
    configGetInt(old, CONFIG_ITEM_CHEATMODE, &gGlobalGameCfg.cheat_mode);
    configGetInt(old, CONFIG_ITEM_ENABLEIMAGE, &gGlobalGameCfg.cheat_enable_image);
#endif
#ifdef PADEMU
    configGetInt(old, CONFIG_ITEM_ENABLEPADEMU, &gGlobalGameCfg.pademu_enable);
    configGetInt(old, CONFIG_ITEM_PADEMUSETTINGS, &gGlobalGameCfg.pademu_settings);
    configGetInt(old, CONFIG_ITEM_PADMACROSETTINGS, &gGlobalGameCfg.padmacro_settings);
#endif
    configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_ENABLE, &gGlobalGameCfg.osd_enable);
    configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_LANGID, &gGlobalGameCfg.osd_langid);
    configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_TV_ASP, &gGlobalGameCfg.osd_tv_aspect);
    configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_VMODE, &gGlobalGameCfg.osd_vmode);
    configClear(old);
    return 1;
}

int cfgMigrateLegacyPerGame(const char *path, per_game_cfg_t *cfg)
{
    config_set_t tmp;
    config_set_t *old = configAlloc(0, &tmp, (char *)path);
    if (!old || !configRead(old)) {
        if (old)
            configClear(old);
        return 0;
    }
    configGetInt(old, CONFIG_ITEM_COMPAT, &cfg->compat);
    configGetInt(old, CONFIG_ITEM_DMA, &cfg->dma);
    configGetInt(old, CONFIG_ITEM_CORE_LOADER, &cfg->core_loader);
    configGetInt(old, CONFIG_ITEM_CONFIGSOURCE, &cfg->config_source);
    configGetStrCopy(old, CONFIG_ITEM_DNAS, cfg->dnas, sizeof(cfg->dnas));
    configGetStrCopy(old, CONFIG_ITEM_ALTSTARTUP, cfg->alt_startup, sizeof(cfg->alt_startup));
    configGetVMC(old, cfg->vmc1, sizeof(cfg->vmc1), 0);
    configGetVMC(old, cfg->vmc2, sizeof(cfg->vmc2), 1);
    const char *str;
    if (configGetStr(old, CONFIG_ITEM_FORMAT, &str))
        strncpy(cfg->format, str, sizeof(cfg->format) - 1);
    if (configGetStr(old, CONFIG_ITEM_MEDIA, &str))
        strncpy(cfg->media, str, sizeof(cfg->media) - 1);
    configGetInt(old, CONFIG_ITEM_SIZE, &cfg->size_mb);
#ifdef GSM
    configGetInt(old, CONFIG_ITEM_GSMSOURCE, &cfg->gsm_source);
    configGetInt(old, CONFIG_ITEM_ENABLEGSM, &cfg->gsm_enable);
    configGetInt(old, CONFIG_ITEM_GSMVMODE, &cfg->gsm_vmode);
    configGetInt(old, CONFIG_ITEM_GSMXOFFSET, &cfg->gsm_xoffset);
    configGetInt(old, CONFIG_ITEM_GSMYOFFSET, &cfg->gsm_yoffset);
    configGetInt(old, CONFIG_ITEM_GSMFIELDFIX, &cfg->gsm_fieldfix);
#endif
#ifdef CHEAT
    configGetInt(old, CONFIG_ITEM_CHEATSSOURCE, &cfg->cheat_source);
    configGetInt(old, CONFIG_ITEM_ENABLECHEAT, &cfg->cheat_enable);
    configGetInt(old, CONFIG_ITEM_CHEATMODE, &cfg->cheat_mode);
    configGetInt(old, CONFIG_ITEM_ENABLEIMAGE, &cfg->cheat_enable_image);
#endif
#ifdef PADEMU
    configGetInt(old, CONFIG_ITEM_PADEMUSOURCE, &cfg->pademu_source);
    configGetInt(old, CONFIG_ITEM_ENABLEPADEMU, &cfg->pademu_enable);
    configGetInt(old, CONFIG_ITEM_PADEMUSETTINGS, &cfg->pademu_settings);
    configGetInt(old, CONFIG_ITEM_PADMACROSOURCE, &cfg->padmacro_source);
    configGetInt(old, CONFIG_ITEM_PADMACROSETTINGS, &cfg->padmacro_settings);
#endif
    configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_SOURCE, &cfg->osd_source);
    configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_ENABLE, &cfg->osd_enable);
    configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_LANGID, &cfg->osd_langid);
    configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_TV_ASP, &cfg->osd_tv_aspect);
    configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_VMODE, &cfg->osd_vmode);
    configClear(old);

    return 1;
}

int cfgMigrateLegacyGameInfo(const char *path, game_info_t *gi)
{
    config_set_t tmp;
    config_set_t *old = configAlloc(0, &tmp, (char *)path);
    if (!old || !configRead(old)) {
        if (old)
            configClear(old);
        return 0;
    }

    const char *str;
    int found = 0;

    if (configGetStr(old, CONFIG_ITEM_NAME, &str)) {
        strncpy(gi->title, str, sizeof(gi->title) - 1);
        found = 1;
    }

    if (configGetStr(old, "Genre", &str)) {
        strncpy(gi->genre, str, sizeof(gi->genre) - 1);
        found = 1;
    }

    if (configGetStr(old, "Release", &str)) {
        strncpy(gi->release, str, sizeof(gi->release) - 1);
        found = 1;
    }

    if (configGetStr(old, "Developer", &str)) {
        strncpy(gi->developer, str, sizeof(gi->developer) - 1);
        found = 1;
    }

    if (configGetStr(old, "Description", &str)) {
        strncpy(gi->description, str, sizeof(gi->description) - 1);
        found = 1;
    }

    if (configGetStr(old, "Publisher", &str)) {
        strncpy(gi->publisher, str, sizeof(gi->publisher) - 1);
        found = 1;
    }

    configClear(old);
    return found;
}

int cfgMigrateTARGameCfg(const char *startup, game_info_t *gi, per_game_cfg_t *pgcfg)
{
    char tarname[32];
    snprintf(tarname, sizeof(tarname), "%s.cfg", startup);
    TarEntryBase *e = tarFind(TAR_KIND_CFG, tarname);
    if (!e)
        return 0;

    void *buf = malloc(e->rawSize);
    if (!buf)
        return 0;

    int loaded = 0;
    if (tarRead(TAR_KIND_CFG, e, buf, e->rawSize) == e->rawSize) {
        config_set_t tmp;
        config_set_t *old = configAlloc(0, &tmp, NULL);
        if (old && configReadBuffer(old, buf, (int)e->rawSize)) {
            const char *str;
            if (gi) {
                if (configGetStr(old, CONFIG_ITEM_NAME, &str))
                    strncpy(gi->title, str, sizeof(gi->title) - 1);
                if (configGetStr(old, "Genre", &str))
                    strncpy(gi->genre, str, sizeof(gi->genre) - 1);
                if (configGetStr(old, "Release", &str))
                    strncpy(gi->release, str, sizeof(gi->release) - 1);
                if (configGetStr(old, "Developer", &str))
                    strncpy(gi->developer, str, sizeof(gi->developer) - 1);
                if (configGetStr(old, "Description", &str))
                    strncpy(gi->description, str, sizeof(gi->description) - 1);

                loaded = 1;
            }

            if (pgcfg) {
                configGetInt(old, CONFIG_ITEM_COMPAT, &pgcfg->compat);
                configGetInt(old, CONFIG_ITEM_DMA, &pgcfg->dma);
                configGetInt(old, CONFIG_ITEM_CORE_LOADER, &pgcfg->core_loader);
                configGetInt(old, CONFIG_ITEM_CONFIGSOURCE, &pgcfg->config_source);
                configGetStrCopy(old, CONFIG_ITEM_DNAS, pgcfg->dnas, sizeof(pgcfg->dnas));
                configGetStrCopy(old, CONFIG_ITEM_ALTSTARTUP, pgcfg->alt_startup, sizeof(pgcfg->alt_startup));
                configGetVMC(old, pgcfg->vmc1, sizeof(pgcfg->vmc1), 0);
                configGetVMC(old, pgcfg->vmc2, sizeof(pgcfg->vmc2), 1);

                if (configGetStr(old, CONFIG_ITEM_FORMAT, &str))
                    strncpy(pgcfg->format, str, sizeof(pgcfg->format) - 1);
                if (configGetStr(old, CONFIG_ITEM_MEDIA, &str))
                    strncpy(pgcfg->media, str, sizeof(pgcfg->media) - 1);
                configGetInt(old, CONFIG_ITEM_SIZE, &pgcfg->size_mb);
#ifdef GSM
                configGetInt(old, CONFIG_ITEM_GSMSOURCE, &pgcfg->gsm_source);
                configGetInt(old, CONFIG_ITEM_ENABLEGSM, &pgcfg->gsm_enable);
                configGetInt(old, CONFIG_ITEM_GSMVMODE, &pgcfg->gsm_vmode);
                configGetInt(old, CONFIG_ITEM_GSMXOFFSET, &pgcfg->gsm_xoffset);
                configGetInt(old, CONFIG_ITEM_GSMYOFFSET, &pgcfg->gsm_yoffset);
                configGetInt(old, CONFIG_ITEM_GSMFIELDFIX, &pgcfg->gsm_fieldfix);
#endif
#ifdef CHEAT
                configGetInt(old, CONFIG_ITEM_CHEATSSOURCE, &pgcfg->cheat_source);
                configGetInt(old, CONFIG_ITEM_ENABLECHEAT, &pgcfg->cheat_enable);
                configGetInt(old, CONFIG_ITEM_CHEATMODE, &pgcfg->cheat_mode);
                configGetInt(old, CONFIG_ITEM_ENABLEIMAGE, &pgcfg->cheat_enable_image);
#endif
#ifdef PADEMU
                configGetInt(old, CONFIG_ITEM_PADEMUSOURCE, &pgcfg->pademu_source);
                configGetInt(old, CONFIG_ITEM_ENABLEPADEMU, &pgcfg->pademu_enable);
                configGetInt(old, CONFIG_ITEM_PADEMUSETTINGS, &pgcfg->pademu_settings);
                configGetInt(old, CONFIG_ITEM_PADMACROSOURCE, &pgcfg->padmacro_source);
                configGetInt(old, CONFIG_ITEM_PADMACROSETTINGS, &pgcfg->padmacro_settings);
#endif
                configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_SOURCE, &pgcfg->osd_source);
                configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_ENABLE, &pgcfg->osd_enable);
                configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_LANGID, &pgcfg->osd_langid);
                configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_TV_ASP, &pgcfg->osd_tv_aspect);
                configGetInt(old, CONFIG_ITEM_OSD_SETTINGS_VMODE, &pgcfg->osd_vmode);
                loaded = 1;
            }
            configClear(old);
        }
    }

    free(buf);
    return loaded;
}

int cfgMigrateLegacyAppTitleCfg(const char *path)
{
    config_set_t tmp;
    config_set_t *old = configAlloc(0, &tmp, (char *)path);
    if (!configRead(old)) {
        configClear(old);
        return 0;
    }

    config_t lcfg;
    config_init(&lcfg);
    config_setting_t *root = config_root_setting(&lcfg);

    const char *fields[] = {
        "title", "boot", "argv1",
        "Title", "Description", "Developer",
        "Version", "Release", "Package", "Source",
        NULL};
    const char *value;
    for (int f = 0; fields[f]; f++) {
        if (configGetStr(old, fields[f], &value)) {
            config_setting_t *s = config_setting_add(root, fields[f], CONFIG_TYPE_STRING);
            if (s)
                config_setting_set_string(s, value);
        }
    }
    configClear(old);
    config_write_file(&lcfg, path);
    config_destroy(&lcfg);
    return 1;
}
