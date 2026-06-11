// ---------------------------------------------------------------------------
// Legacy migration
//
// Called when libconfig fails to parse the file.. the file is in the old
// key=value format.. uses the existing config.c to read all
// values into globals, then wOPLSave() immediately rewrites them in
// the new libconfig format.. phase out eventually
// ---------------------------------------------------------------------------
// DELETE_WITH_MIGRATION whole file

#include "include/common.h"
#include "include/config_wopl.h"
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
#include "include/config_migration.h"
#include "include/tar.h"

#include <libconfig.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include <fcntl.h>
#include <unistd.h>

#ifdef __DEBUG
#include "include/debug.h"
#endif

#ifdef GSM
#include "include/pggsm.h"
#endif

#ifdef CHEAT
#include "include/cheatman.h"
#endif

static int configReadLegacyIP(void);

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

    configGetStrCopy(cfg, CONFIG_OPL_PARENTAL_LOCK_PWD, gParentalLockPassword, sizeof(gParentalLockPassword));

    int delay;
    if (configGetInt(cfg, "fav_frames_delay", &delay))
        gFAVFramesDelay = delay;
    if (configGetInt(cfg, "usb_frames_delay", &delay)) {
        gBDMFramesDelay = delay;
        gMMCEFramesDelay = delay;
    }
    if (configGetInt(cfg, "app_frames_delay", &delay))
        gAPPFramesDelay = delay;
    if (configGetInt(cfg, "eth_frames_delay", &delay))
        gETHFramesDelay = delay;
    if (configGetInt(cfg, "hdd_frames_delay", &delay))
        gHDDFramesDelay = delay;

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
    configGetStrCopy(old, CONFIG_ITEM_DNAS, cfg->dnas, sizeof(cfg->dnas));
    configGetStrCopy(old, CONFIG_ITEM_ALTSTARTUP, cfg->alt_startup, sizeof(cfg->alt_startup));
    configGetVMC(old, cfg->vmc1, sizeof(cfg->vmc1), 0);
    configGetVMC(old, cfg->vmc2, sizeof(cfg->vmc2), 1);
    const char *str;
    if (configGetStr(old, CONFIG_ITEM_FORMAT, &str)) {
        strncpy(cfg->format, str, sizeof(cfg->format) - 1);
        cfg->format[sizeof(cfg->format) - 1] = '\0';
    }

    if (configGetStr(old, CONFIG_ITEM_MEDIA, &str)) {
        strncpy(cfg->media, str, sizeof(cfg->media) - 1);
        cfg->media[sizeof(cfg->media) - 1] = '\0';
    }

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
        gi->title[sizeof(gi->title) - 1] = '\0';
        found = 1;
    }

    if (configGetStr(old, "Genre", &str)) {
        strncpy(gi->genre, str, sizeof(gi->genre) - 1);
        gi->genre[sizeof(gi->genre) - 1] = '\0';
        found = 1;
    }

    if (configGetStr(old, "Release", &str)) {
        strncpy(gi->release, str, sizeof(gi->release) - 1);
        gi->release[sizeof(gi->release) - 1] = '\0';
        found = 1;
    }

    if (configGetStr(old, "Developer", &str)) {
        strncpy(gi->developer, str, sizeof(gi->developer) - 1);
        gi->developer[sizeof(gi->developer) - 1] = '\0';
        found = 1;
    }

    if (configGetStr(old, "Description", &str)) {
        strncpy(gi->description, str, sizeof(gi->description) - 1);
        gi->description[sizeof(gi->description) - 1] = '\0';
        found = 1;
    }

    if (configGetStr(old, "Publisher", &str)) {
        strncpy(gi->publisher, str, sizeof(gi->publisher) - 1);
        gi->publisher[sizeof(gi->publisher) - 1] = '\0';
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
                if (configGetStr(old, CONFIG_ITEM_NAME, &str)) {
                    strncpy(gi->title, str, sizeof(gi->title) - 1);
                    gi->title[sizeof(gi->title) - 1] = '\0';
                }
                if (configGetStr(old, "Genre", &str)) {
                    strncpy(gi->genre, str, sizeof(gi->genre) - 1);
                    gi->genre[sizeof(gi->genre) - 1] = '\0';
                }
                if (configGetStr(old, "Release", &str)) {
                    strncpy(gi->release, str, sizeof(gi->release) - 1);
                    gi->release[sizeof(gi->release) - 1] = '\0';
                }
                if (configGetStr(old, "Developer", &str)) {
                    strncpy(gi->developer, str, sizeof(gi->developer) - 1);
                    gi->developer[sizeof(gi->developer) - 1] = '\0';
                }
                if (configGetStr(old, "Description", &str)) {
                    strncpy(gi->description, str, sizeof(gi->description) - 1);
                    gi->description[sizeof(gi->description) - 1] = '\0';
                }

                loaded = 1;
            }

            if (pgcfg) {
                configGetInt(old, CONFIG_ITEM_COMPAT, &pgcfg->compat);
                configGetInt(old, CONFIG_ITEM_DMA, &pgcfg->dma);
                configGetInt(old, CONFIG_ITEM_CORE_LOADER, &pgcfg->core_loader);
                configGetStrCopy(old, CONFIG_ITEM_DNAS, pgcfg->dnas, sizeof(pgcfg->dnas));
                configGetStrCopy(old, CONFIG_ITEM_ALTSTARTUP, pgcfg->alt_startup, sizeof(pgcfg->alt_startup));
                configGetVMC(old, pgcfg->vmc1, sizeof(pgcfg->vmc1), 0);
                configGetVMC(old, pgcfg->vmc2, sizeof(pgcfg->vmc2), 1);

                if (configGetStr(old, CONFIG_ITEM_FORMAT, &str)) {
                    strncpy(pgcfg->format, str, sizeof(pgcfg->format) - 1);
                    pgcfg->format[sizeof(pgcfg->format) - 1] = '\0';
                }
                if (configGetStr(old, CONFIG_ITEM_MEDIA, &str)) {
                    strncpy(pgcfg->media, str, sizeof(pgcfg->media) - 1);
                    pgcfg->media[sizeof(pgcfg->media) - 1] = '\0';
                }

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
    if (!old)
        return 0;

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
    char bak[256];
    snprintf(bak, sizeof(bak), "%s.bak", path);
    rename(path, bak);
    int ok = config_write_file(&lcfg, path);
    config_destroy(&lcfg);

    return ok;
}

int cfgMigrateLegacyTheme(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return 0;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    if (sz <= 0) {
        fclose(f);
        return 0;
    }

    char *buf = malloc(sz);
    if (!buf) {
        fclose(f);
        return 0;
    }

    if (fread(buf, 1, sz, f) != (size_t)sz) {
        free(buf);
        fclose(f);
        return 0;
    }
    fclose(f);

    config_t cfg;
    config_init(&cfg);
    config_setting_t *root = config_root_setting(&cfg);
    config_setting_t *section = NULL;
    const char *p = buf;
    const char *end = buf + sz;

    while (p < end) {
        const char *lend = p;
        while (lend < end && *lend != '\n')
            lend++;
        int len = (int)(lend - p);
        char line[512];
        if (len >= (int)sizeof(line))
            len = sizeof(line) - 1;
        memcpy(line, p, len);
        while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n'))
            len--;
        line[len] = '\0';
        p = (lend < end) ? lend + 1 : end;

        if (!line[0] || line[0] == '#')
            continue;

        if (line[0] == '\t') {
            if (!section)
                continue;
            char *eq = strchr(line + 1, '=');
            if (!eq)
                continue;
            *eq = '\0';
            char *key = line + 1;
            char *val = eq + 1;
            char *endp;
            long ival = strtol(val, &endp, 10);
            if (endp != val && *endp == '\0') {
                config_setting_t *s = config_setting_add(section, key, CONFIG_TYPE_INT);
                if (s)
                    config_setting_set_int(s, (int)ival);
            } else {
                config_setting_t *s = config_setting_add(section, key, CONFIG_TYPE_STRING);
                if (s)
                    config_setting_set_string(s, val);
            }
        } else {
            if (len > 0 && line[len - 1] == ':') {
                line[len - 1] = '\0';
                section = config_setting_add(root, line, CONFIG_TYPE_GROUP);
            } else {
                char *eq = strchr(line, '=');
                if (!eq)
                    continue;
                *eq = '\0';
                char *key = line;
                char *val = eq + 1;
                char *endp;
                long ival = strtol(val, &endp, 10);
                if (endp != val && *endp == '\0') {
                    config_setting_t *s = config_setting_add(root, key, CONFIG_TYPE_INT);
                    if (s)
                        config_setting_set_int(s, (int)ival);
                } else {
                    config_setting_t *s = config_setting_add(root, key, CONFIG_TYPE_STRING);
                    if (s)
                        config_setting_set_string(s, val);
                }
            }
        }
    }

    free(buf);
    char bak[256];
    snprintf(bak, sizeof(bak), "%s.bak", path);
    rename(path, bak);
    int ok = config_write_file(&cfg, path);
    config_destroy(&cfg);

    return ok;
}

int cfgBatchMigratePerGame(const char *inputPrefix, const char *outputPrefix, int keepOriginals)
{
    char cfgDir[256];
    snprintf(cfgDir, sizeof(cfgDir), "%sCFG", inputPrefix);

    DIR *dir = opendir(cfgDir);
    if (!dir)
        return 0;

    if (strcmp(inputPrefix, outputPrefix) != 0) {
        char outDir[256];
        snprintf(outDir, sizeof(outDir), "%sCFG", outputPrefix);
        mkdir(outDir, 0777);
    }

    int count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        int len = strlen(entry->d_name);
        if (len < 5 || strcasecmp(entry->d_name + len - 4, ".cfg") != 0)
            continue;

        char inputPath[256], outputCfgPath[256], outputInfoPath[256];
        snprintf(inputPath, sizeof(inputPath), "%sCFG/%s", inputPrefix, entry->d_name);
        snprintf(outputCfgPath, sizeof(outputCfgPath), "%sCFG/%s", outputPrefix, entry->d_name);

        // info path.. same basename .info extension
        char basename[128];
        int baseLen = len - 4;
        strncpy(basename, entry->d_name, baseLen);
        basename[baseLen] = '\0';
        snprintf(outputInfoPath, sizeof(outputInfoPath), "%sCFG/%s.info", outputPrefix, basename);

        // only process legacy format files
        per_game_cfg_t pgcfg;

        // skip if already new format.. wOPLPerGameLoad also inits pgcfg
        if (wOPLPerGameLoad(inputPath, &pgcfg) == 1)
            continue;

        // skip if not a readable legacy format either
        if (!cfgMigrateLegacyPerGame(inputPath, &pgcfg))
            continue;

        // extract game info from the legacy cfg
        game_info_t gi;
        memset(&gi, 0, sizeof(gi));
        int hasInfo = cfgMigrateLegacyGameInfo(inputPath, &gi);

        int samePath = (strcmp(inputPath, outputCfgPath) == 0);
        if (keepOriginals && samePath) {
            char bakPath[256];
            snprintf(bakPath, sizeof(bakPath), "%s.bak", inputPath);
            rename(inputPath, bakPath);
        }

        if (wOPLPerGameSave(outputCfgPath, &pgcfg)) {
            // only write .info if we got data and one doesn't already exist
            if (hasInfo) {
                FILE *f = fopen(outputInfoPath, "r");
                int infoExists = (f != NULL);
                if (f) fclose(f);
                if (!infoExists)
                    wOPLGameInfoSave(outputInfoPath, &gi);
            }
            if (!keepOriginals && !samePath)
                unlink(inputPath);
            count++;
        } else if (keepOriginals && samePath) {
            // cfg save failed.. restore .bak
            char bakPath[256];
            snprintf(bakPath, sizeof(bakPath), "%s.bak", inputPath);
            rename(bakPath, inputPath);
        }
    }

    closedir(dir);

    return count;
}

// ---------------------------------------------------------------------------
// DELETE_WITH_MIGRATION.. legacy key=value config infrastructure
// Everything in this file is only needed during the migration period
// ---------------------------------------------------------------------------

static u32 currentUID = 0;
static config_set_t configFiles[CONFIG_INDEX_COUNT];
static char legacyNetConfigPath[256] = "mc?:SYS-CONF/IPCONFIG.DAT";
static const char *configFilenames[CONFIG_INDEX_COUNT] = {
    "conf_wopl.cfg",
    "conf_last.cfg",
    "conf_apps.cfg",
    "conf_network.cfg",
    "conf_game.cfg",
};

static char cfgDevice[128];

static int strToColor(const char *string, unsigned char *color)
{
    int cnt = 0, n = 0;
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;

    if (!string || !*string)
        return 0;
    if (string[0] != '#')
        return 0;

    string++;

    while (*string) {
        int fh = fromHex(*string);
        if (fh >= 0) {
            color[n] = color[n] * 16 + fh;
        } else {
            break;
        }

        // Two characters per color
        if (cnt == 1) {
            cnt = 0;
            n++;
        } else {
            cnt++;
        }

        string++;
    }

    return 1;
}

/// true if given a whitespace character
int isWS(char c)
{
    return c == ' ' || c == '\t';
}

static int splitAssignment(char *line, char *key, size_t keymax, char *val, size_t valmax)
{
    // skip whitespace
    for (; isWS(*line); ++line)
        ;

    // find "=".
    // If found, the text before is key, after is val.
    // Otherwise malformed string is encountered

    char *eqpos = strchr(line, '=');

    if (eqpos) {
        // copy the name and the value
        size_t keylen = MIN(keymax, eqpos - line);

        strncpy(key, line, keylen);

        eqpos++;

        size_t vallen = MIN(valmax, strlen(line) - (eqpos - line));
        strncpy(val, eqpos, vallen);
    }

    return (int)eqpos;
}

static int parsePrefix(char *line, char *prefix)
{
    // find "=".
    // If found, the text before is key, after is val.
    // Otherwise malformed string is encountered
    char *colpos = strchr(line, ':');

    if (colpos && colpos != line) {
        // copy the name and the value
        strncpy(prefix, line, colpos - line);
        prefix[colpos - line] = 0;

        return 1;
    } else {
        return 0;
    }
}

static int configKeyValidate(const char *key)
{
    if (strlen(key) == 0)
        return 0;

    return !strchr(key, '=');
}

static struct config_kv_t *allocConfigItem(const char *key, const char *val)
{
    struct config_kv_t *it = (struct config_kv_t *)malloc(sizeof(struct config_kv_t));
    strncpy(it->key, key, sizeof(it->key));
    it->key[sizeof(it->key) - 1] = '\0';
    strncpy(it->val, val, sizeof(it->val));
    it->val[sizeof(it->val) - 1] = '\0';
    it->next = NULL;

    return it;
}

/// Low level key addition. Does not check for uniqueness.
static void addConfigValue(config_set_t *configSet, const char *key, const char *val)
{
    if (!configSet->tail) {
        configSet->head = allocConfigItem(key, val);
        configSet->tail = configSet->head;
    } else {
        configSet->tail->next = allocConfigItem(key, val);
        configSet->tail = configSet->tail->next;
    }
}

static struct config_kv_t *getConfigItemForName(config_set_t *configSet, const char *name)
{
    struct config_kv_t *val = configSet->head;

    while (val) {
        if (strncmp(val->key, name, sizeof(val->key)) == 0)
            break;

        val = val->next;
    }

    return val;
}

static int configReadFileBuffer(file_buffer_t *fileBuffer, config_set_t *configSet)
{
    char *line;
    unsigned int lineno = 0;

    char prefix[CONFIG_KEY_NAME_LEN];
    memset(prefix, 0, sizeof(prefix));

    while (sbReadFileBuffer(fileBuffer, &line)) {
        lineno++;

        char key[CONFIG_KEY_NAME_LEN], val[CONFIG_KEY_VALUE_LEN];
        memset(key, 0, sizeof(key));
        memset(val, 0, sizeof(val));

        if (splitAssignment(line, key, sizeof(key), val, sizeof(val))) {
            /* if the line does not start with whitespace,
             * the prefix ends and we have to reset it
             */
            if (!isWS(line[0]))
                memset(prefix, 0, sizeof(prefix));

            // insert config value
            if (prefix[0]) {
                // we have a prefix
                char composedKey[CONFIG_KEY_NAME_LEN];

                snprintf(composedKey, sizeof(composedKey), "%s_%s", prefix, key);
                configSetStr(configSet, composedKey, val);
            } else {
                configSetStr(configSet, key, val);
            }
        } else if (parsePrefix(line, prefix)) {
            // prefix is set, that's about it
        } else {
            LOG("CONFIG Malformed file '%s' line %d: '%s'\n", configSet->filename, lineno, line);
        }
    }
    configSet->modified = 0;
    return 1;
}

config_set_t *configAlloc(int type, config_set_t *configSet, char *fileName)
{
    if (!configSet)
        configSet = (config_set_t *)malloc(sizeof(config_set_t));

    configSet->uid = ++currentUID;
    configSet->type = type;
    configSet->head = NULL;
    configSet->tail = NULL;
    if (fileName) {
        int length = strlen(fileName) + 1;
        configSet->filename = (char *)malloc(length * sizeof(char));
        memcpy(configSet->filename, fileName, length);
    } else
        configSet->filename = NULL;
    configSet->modified = 0;
    return configSet;
}

void configMove(config_set_t *configSet, const char *fileName)
{
    int length = strlen(fileName) + 1;
    configSet->filename = realloc(configSet->filename, length);
    memcpy(configSet->filename, fileName, length);
}

void configFree(config_set_t *configSet)
{
    configClear(configSet);
    free(configSet->filename);
    free(configSet);
}

void configClear(config_set_t *configSet)
{
    while (configSet->head) {
        struct config_kv_t *cur = configSet->head;
        configSet->head = cur->next;

        free(cur);
    }

    configSet->head = NULL;
    configSet->tail = NULL;
    configSet->modified = 1;
}

config_set_t *configGetByType(int type)
{
    int index = 0;
    while (index < CONFIG_INDEX_COUNT) {
        config_set_t *configSet = &configFiles[index];

        if (configSet->type == type)
            return configSet;
        index++;
    }
    return NULL;
}

int configSetStr(config_set_t *configSet, const char *key, const char *value)
{
    if (!configKeyValidate(key))
        return 0;

    struct config_kv_t *it = getConfigItemForName(configSet, key);

    if (it) {
        if (strncmp(it->val, value, sizeof(it->val)) != 0) {
            strncpy(it->val, value, sizeof(it->val));
            it->val[sizeof(it->val) - 1] = '\0';
            if (it->key[0] != '#')
                configSet->modified = 1;
        }
    } else {
        addConfigValue(configSet, key, value);
        if (key[0] != '#')
            configSet->modified = 1;
    }

    return 1;
}

// sets the value to point to the value str in the config. Do not overwrite - it will overwrite the string in config
int configGetStr(config_set_t *configSet, const char *key, const char **value)
{
    if (!configKeyValidate(key))
        return 0;

    struct config_kv_t *it = getConfigItemForName(configSet, key);

    if (it) {
        *value = it->val;
        return 1;
    } else
        return 0;
}

int configGetStrCopy(config_set_t *configSet, const char *key, char *value, int length)
{
    const char *valref = NULL;
    if (configGetStr(configSet, key, &valref)) {
        strncpy(value, valref, length);
        value[length - 1] = '\0';
        return 1;
    } else {
        value[0] = '\0';
        return 0;
    }
}

int configSetInt(config_set_t *configSet, const char *key, const int value)
{
    char tmp[12];
    snprintf(tmp, sizeof(tmp), "%d", value);
    return configSetStr(configSet, key, tmp);
}

int configGetInt(config_set_t *configSet, const char *key, int *value)
{
    const char *valref = NULL;
    if (configGetStr(configSet, key, &valref)) {
        *value = atoi(valref);
        return 1;
    } else {
        return 0;
    }
}

int configSetColor(config_set_t *configSet, const char *key, unsigned char *color)
{
    char tmp[8];
    snprintf(tmp, sizeof(tmp), "#%02X%02X%02X", color[0], color[1], color[2]);
    return configSetStr(configSet, key, tmp);
}

int configGetColor(config_set_t *configSet, const char *key, unsigned char *color)
{
    const char *valref = NULL;
    if (configGetStr(configSet, key, &valref)) {
        strToColor(valref, color);
        return 1;
    } else {
        return 0;
    }
}

int configRemoveKey(config_set_t *configSet, const char *key)
{
    if (!configKeyValidate(key))
        return 0;

    struct config_kv_t *val = configSet->head;
    struct config_kv_t *prev = NULL;

    while (val) {
        if (strncmp(val->key, key, sizeof(val->key)) == 0) {
            if (key[0] != '#')
                configSet->modified = 1;

            if (val == configSet->tail)
                configSet->tail = prev;

            val = val->next;
            if (prev) {
                free(prev->next);
                prev->next = val;
            } else {
                free(configSet->head);
                configSet->head = val;
            }
        } else {
            prev = val;
            val = val->next;
        }
    }

    return 1;
}

void configGetVMC(config_set_t *configSet, char *vmc, int length, int slot)
{
    char gkey[CONFIG_KEY_NAME_LEN];
    snprintf(gkey, sizeof(gkey), "%s_%d", CONFIG_ITEM_VMC, slot);
    configGetStrCopy(configSet, gkey, vmc, length);
}

void configSetVMC(config_set_t *configSet, const char *vmc, int slot)
{
    char gkey[CONFIG_KEY_NAME_LEN];
    if (vmc[0] == '\0') {
        configRemoveVMC(configSet, slot);
        return;
    }
    snprintf(gkey, sizeof(gkey), "%s_%d", CONFIG_ITEM_VMC, slot);
    configSetStr(configSet, gkey, vmc);
}

void configRemoveVMC(config_set_t *configSet, int slot)
{
    char gkey[CONFIG_KEY_NAME_LEN];
    snprintf(gkey, sizeof(gkey), "%s_%d", CONFIG_ITEM_VMC, slot);
    configRemoveKey(configSet, gkey);
}

int configReadBuffer(config_set_t *configSet, const void *buffer, int size)
{
    int ret;
    file_buffer_t *fileBuffer = sbOpenFileBufferBuffer(0, buffer, size);
    if (!fileBuffer) {
        configSet->modified = 0;
        return 0;
    }

    ret = configReadFileBuffer(fileBuffer, configSet);

    sbCloseFileBuffer(fileBuffer);
    return ret;
}

int configRead(config_set_t *configSet)
{
    int ret;
    file_buffer_t *fileBuffer = sbOpenFileBuffer(configSet->filename, O_RDONLY, 0, 4096);
    if (!fileBuffer) {
        LOG("CONFIG No file %s.\n", configSet->filename);
        configSet->modified = 0;
        return 0;
    }

    ret = configReadFileBuffer(fileBuffer, configSet);

    sbCloseFileBuffer(fileBuffer);
    return ret;
}

int configReadMulti(int types)
{
    int result = 0, index;

    for (index = 0; index < CONFIG_INDEX_COUNT; index++) {
        config_set_t *configSet = &configFiles[index];

        if (configSet->type & types) {
            configClear(configSet);
            if (configRead(configSet))
                result |= configSet->type;
        }
    }

    // If the network configuration is to be loaded and one cannot be loaded, attempt to load from the legacy network config file.
    if ((types & CONFIG_NETWORK) && !(result & CONFIG_NETWORK))
        if (configReadLegacyIP())
            result |= CONFIG_NETWORK;

    return result;
}

int configWrite(config_set_t *configSet)
{
    if (configSet->modified) {
        file_buffer_t *fileBuffer = sbOpenFileBuffer(configSet->filename, O_WRONLY | O_CREAT | O_TRUNC, 0, 4096);
        if (fileBuffer) {
            char line[512];

            bgmMute();
            struct config_kv_t *cur = configSet->head;
            while (cur) {
                if ((cur->key[0] != '\0') && (cur->key[0] != '#')) {
                    snprintf(line, sizeof(line), "%s=%s\r\n", cur->key, cur->val); // add windows CR+LF (0x0D 0x0A)
                    sbWriteFileBuffer(fileBuffer, line, strlen(line));
                }

                // and advance
                cur = cur->next;
            }

            sbCloseFileBuffer(fileBuffer);
            configSet->modified = 0;
            bgmUnMute();
            return 1;
        }
        return 0;
    }
    return 1;
}

int configWriteMulti(int types)
{
    int result = 0, index;

    for (index = 0; index < CONFIG_INDEX_COUNT; index++) {
        config_set_t *configSet = &configFiles[index];

        if (configSet->type & types)
            result += configWrite(configSet);
    }

    return result;
}

void configInit(char *prefix)
{
    char path[256];
    int i;

    if (prefix)
        snprintf(legacyNetConfigPath, sizeof(legacyNetConfigPath), "%s/IPCONFIG.DAT", prefix);
    else
        prefix = gBaseMCDir;

    for (i = 0; i < CONFIG_INDEX_COUNT; i++) {
        snprintf(path, sizeof(path), "%s/%s", prefix, configFilenames[i]);
        configAlloc(1 << i, &configFiles[i], path);
    }

    configPrepareNotifications(prefix);
}

void configSetMove(char *prefix)
{
    char path[256];
    int i;

    if (prefix)
        snprintf(legacyNetConfigPath, sizeof(legacyNetConfigPath), "%s/IPCONFIG.DAT", prefix);
    else
        prefix = gBaseMCDir;

    for (i = 0; i < CONFIG_INDEX_COUNT; i++) {
        snprintf(path, sizeof(path), "%s/%s", prefix, configFilenames[i]);
        configMove(&configFiles[i], path);
    }

    configPrepareNotifications(prefix);
}

void configEnd()
{
    int index = 0;
    while (index < CONFIG_INDEX_COUNT) {
        config_set_t *configSet = &configFiles[index];

        configClear(configSet);
        free(configSet->filename);
        configSet->filename = NULL;
        index++;
    }
}

char *configGetDir(void)
{
    static char path[256];

    if (!strncmp(cfgDevice, "mc", 2))
        snprintf(path, sizeof(path), "mc%d:%s/", sbGetmcID() & 1, WOPL_CONFIG_NAME);
    else
        snprintf(path, sizeof(path), "%s", cfgDevice);

    return path;
}

void configPrepareNotifications(char *prefix)
{
    snprintf(cfgDevice, sizeof(cfgDevice), prefix);
}

static int configReadLegacyIP(void)
{
    config_set_t *configSet;
    char temp[16];

    int fd = sbOpenFile(legacyNetConfigPath, O_RDONLY);
    if (fd >= 0) {
        char ipconfig[256];
        int size = sbGetFileSize(fd);
        read(fd, &ipconfig, size);
        close(fd);

        sscanf(ipconfig, "%d.%d.%d.%d %d.%d.%d.%d %d.%d.%d.%d", &ps2_ip[0], &ps2_ip[1], &ps2_ip[2], &ps2_ip[3],
               &ps2_netmask[0], &ps2_netmask[1], &ps2_netmask[2], &ps2_netmask[3],
               &ps2_gateway[0], &ps2_gateway[1], &ps2_gateway[2], &ps2_gateway[3]);

        configSet = &configFiles[CONFIG_INDEX_NETWORK];

        snprintf(temp, sizeof(temp), "%d.%d.%d.%d", ps2_ip[0], ps2_ip[1], ps2_ip[2], ps2_ip[3]);
        configSetStr(configSet, CONFIG_NET_PS2_IP, temp);
        snprintf(temp, sizeof(temp), "%d.%d.%d.%d", ps2_netmask[0], ps2_netmask[1], ps2_netmask[2], ps2_netmask[3]);
        configSetStr(configSet, CONFIG_NET_PS2_NETM, temp);
        snprintf(temp, sizeof(temp), "%d.%d.%d.%d", ps2_gateway[0], ps2_gateway[1], ps2_gateway[2], ps2_gateway[3]);
        configSetStr(configSet, CONFIG_NET_PS2_GATEW, temp);
        // The legacy format has no setting for the DNS server, so duplicate the gateway address.
        configSetStr(configSet, CONFIG_NET_PS2_DNS, temp);

        return 1;
    }

    return 0;
}
