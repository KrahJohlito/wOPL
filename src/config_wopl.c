#include "include/config_wopl.h"
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

#include <libconfig.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef __DEBUG
#include "include/debug.h"
#endif

static char settings_config_dir[128] = {0};

#define WOPL_FILENAME "conf_wopl.cfg"

static void color_to_str(const unsigned char *color, char *out, size_t len)
{
    snprintf(out, len, "#%02X%02X%02X", color[0], color[1], color[2]);
}

static void str_to_color(const char *str, unsigned char *color)
{
    color[0] = color[1] = color[2] = 0;
    if (!str || str[0] != '#')
        return;

    str++;
    for (int i = 0; i < 3; i++) {
        int hi = fromHex(*str++);
        int lo = fromHex(*str++);
        if (hi < 0 || lo < 0)
            return;

        color[i] = (unsigned char)((hi << 4) | lo);
    }
}

static int lookup_int(config_t *cfg, const char *path, int def)
{
    const config_setting_t *setting = config_lookup(cfg, path);
    if (!setting)
        return def;

    switch (config_setting_type(setting)) {
        case CONFIG_TYPE_INT:
            return config_setting_get_int(setting);
        case CONFIG_TYPE_BOOL:
            return config_setting_get_bool(setting) ? 1 : 0;
        default:
            return def;
    }
}

static int lookup_bool(config_t *cfg, const char *path, int def)
{
    return lookup_int(cfg, path, def) != 0;
}

static const char *lookup_str(config_t *cfg, const char *path, const char *def)
{
    const char *val = NULL;
    if (config_lookup_string(cfg, path, &val))
        return val;
    return def;
}

static config_setting_t *add_group(config_setting_t *parent, const char *name)
{
    return config_setting_add(parent, name, CONFIG_TYPE_GROUP);
}

static void set_int(config_setting_t *group, const char *name, int val)
{
    config_setting_t *setting = config_setting_add(group, name, CONFIG_TYPE_INT);
    if (setting)
        config_setting_set_int(setting, val);
}

static void set_bool(config_setting_t *group, const char *name, int val)
{
    config_setting_t *setting = config_setting_add(group, name, CONFIG_TYPE_BOOL);
    if (setting)
        config_setting_set_bool(setting, val ? CONFIG_TRUE : CONFIG_FALSE);
}

static void set_str(config_setting_t *group, const char *name, const char *val)
{
    config_setting_t *setting = config_setting_add(group, name, CONFIG_TYPE_STRING);
    if (setting)
        config_setting_set_string(setting, val ? val : "");
}

static void set_color(config_setting_t *group, const char *name, const unsigned char *color)
{
    char buf[8];
    color_to_str(color, buf, sizeof(buf));
    set_str(group, name, buf);
}

static int file_exists(const char *path)
{
    FILE *fd = fopen(path, "r");
    if (!fd)
        return 0;

    fclose(fd);
    return 1;
}

static int ensure_mc_dir(const char *dir)
{
    struct stat st;
    if (stat(dir, &st) == 0)
        return 1;

    return mkdir(dir, 0777) == 0 || errno == EEXIST;
}

static int probe_config_path(char *dir_out, size_t dir_len, char *path_out, size_t path_len, int for_write)
{
    char dir[128];
    char path[256];

    // 1. MC
    int mc = sysCheckMC();
    if (mc >= 0) {
        snprintf(dir, sizeof(dir), "mc%d:wOPL/", mc & 1);
        snprintf(path, sizeof(path), "%s%s", dir, WOPL_FILENAME);
        if (for_write || file_exists(path)) {
            strncpy(dir_out, dir, dir_len - 1);
            dir_out[dir_len - 1] = '\0';

            strncpy(path_out, path, path_len - 1);
            path_out[path_len - 1] = '\0';

            return 1;
        }
    }

    // 2. BDM.. just 0-1 for now
    const char *mass_dirs[] = {"mass0:/", "mass1:/", NULL};
    for (int i = 0; mass_dirs[i]; i++) {
        snprintf(path, sizeof(path), "%s%s", mass_dirs[i], WOPL_FILENAME);
        if (for_write || file_exists(path)) {
            strncpy(dir_out, mass_dirs[i], dir_len - 1);
            dir_out[dir_len - 1] = '\0';

            strncpy(path_out, path, path_len - 1);
            path_out[path_len - 1] = '\0';

            return 1;
        }
    }

    // 3. HDD.. gHDDPrefix is a char*.. null-check before use.
    if (gHDDPrefix && gHDDPrefix[0]) {
        snprintf(path, sizeof(path), "%s%s", gHDDPrefix, WOPL_FILENAME);
        if (for_write || file_exists(path)) {
            strncpy(dir_out, gHDDPrefix, dir_len - 1);
            dir_out[dir_len - 1] = '\0';

            strncpy(path_out, path, path_len - 1);
            path_out[path_len - 1] = '\0';

            return 1;
        }
    }

    return 0;
}

static void parse_display(config_t *cfg)
{
    gWideScreen = lookup_bool(cfg, "display.widescreen", gWideScreen);
    gVMode = lookup_int(cfg, "display.vmode", gVMode);
    gXOff = lookup_int(cfg, "display.x_offset", gXOff);
    gYOff = lookup_int(cfg, "display.y_offset", gYOff);
    gOverscan = lookup_int(cfg, "display.overscan", gOverscan);
    gScrollSpeed = lookup_int(cfg, "display.scroll_speed", gScrollSpeed);

    const char *color;
    if ((color = lookup_str(cfg, "display.bg_color", NULL)))
        str_to_color(color, gDefaultBgColor);
    if ((color = lookup_str(cfg, "display.text_color", NULL)))
        str_to_color(color, gDefaultTextColor);
    if ((color = lookup_str(cfg, "display.ui_text_color", NULL)))
        str_to_color(color, gDefaultUITextColor);
    if ((color = lookup_str(cfg, "display.sel_text_color", NULL)))
        str_to_color(color, gDefaultSelTextColor);
    if ((color = lookup_str(cfg, "display.plasma_blend_color", NULL)))
        str_to_color(color, gDefaultPlasmaBlendColor);
}

static void parse_ui(config_t *cfg, int *out_theme_id, int *out_lang_id)
{
    const char *theme_name = lookup_str(cfg, "ui.theme", NULL);
    if (theme_name)
        *out_theme_id = thmFindGuiID(theme_name);

    const char *lang_name = lookup_str(cfg, "ui.language", NULL);
    if (lang_name)
        *out_lang_id = lngFindGuiID(lang_name);

    gSelectButton = lookup_bool(cfg, "ui.swap_button", 0) ? KEY_CROSS : KEY_CIRCLE;
    gXSensitivity = lookup_int(cfg, "ui.x_sensitivity", gXSensitivity);
    gYSensitivity = lookup_int(cfg, "ui.y_sensitivity", gYSensitivity);
    gEnableNotifications = lookup_bool(cfg, "ui.notifications", gEnableNotifications);
    gDiscEnableArt = lookup_bool(cfg, "ui.disc_art", gDiscEnableArt);
}

static void parse_audio(config_t *cfg)
{
    gEnableSFX = lookup_bool(cfg, "audio.sfx", gEnableSFX);
    gSFXVolume = lookup_int(cfg, "audio.sfx_volume", gSFXVolume);
    gEnableBootSND = lookup_bool(cfg, "audio.boot_sound", gEnableBootSND);
    gBootSndVolume = lookup_int(cfg, "audio.boot_volume", gBootSndVolume);
    gEnableBGM = lookup_bool(cfg, "audio.bgm", gEnableBGM);
    gBGMVolume = lookup_int(cfg, "audio.bgm_volume", gBGMVolume);

    const char *path = lookup_str(cfg, "audio.bgm_path", NULL);
    if (path)
        strncpy(gDefaultBGMPath, path, sizeof(gDefaultBGMPath) - 1);
}

static void parse_startup(config_t *cfg)
{
    gDefaultDevice = lookup_int(cfg, "startup.default_device", gDefaultDevice);
    gAutosort = lookup_bool(cfg, "startup.auto_sort", gAutosort);
    gAutoRefresh = lookup_bool(cfg, "startup.auto_refresh", gAutoRefresh);
    gRememberLastPlayed = lookup_bool(cfg, "startup.remember_last", gRememberLastPlayed);
    gAutoStartLastPlayed = lookup_bool(cfg, "startup.autostart_last", gAutoStartLastPlayed);
    gBDMStartMode = lookup_int(cfg, "startup.bdm_start_mode", gBDMStartMode);
    gHDDStartMode = lookup_int(cfg, "startup.hdd_start_mode", gHDDStartMode);
    gETHStartMode = lookup_int(cfg, "startup.eth_start_mode", gETHStartMode);
    gAPPStartMode = lookup_int(cfg, "startup.app_start_mode", gAPPStartMode);
    gFAVStartMode = lookup_int(cfg, "startup.fav_start_mode", gFAVStartMode);
    gMMCEStartMode = lookup_int(cfg, "startup.mmce_start_mode", gMMCEStartMode);

    const char *path = lookup_str(cfg, "startup.exit_path", NULL);
    if (path)
        strncpy(gExitPath, path, sizeof(gExitPath) - 1);
}

static void parse_devices(config_t *cfg)
{
    gEnableUSB = lookup_bool(cfg, "devices.usb_enabled", gEnableUSB);
    gEnableILK = lookup_bool(cfg, "devices.ilink_enabled", gEnableILK);
    gEnableMX4SIO = lookup_bool(cfg, "devices.mx4sio_enabled", gEnableMX4SIO);
    gEnableBdmHDD = lookup_bool(cfg, "devices.bdm_hdd_enabled", gEnableBdmHDD);
    bdmCacheSize = lookup_int(cfg, "devices.bdm_cache", bdmCacheSize);
    hddCacheSize = lookup_int(cfg, "devices.hdd_cache", hddCacheSize);
    smbCacheSize = lookup_int(cfg, "devices.smb_cache", smbCacheSize);
    gHDDSpindown = lookup_int(cfg, "devices.hdd_spindown", gHDDSpindown);
    gHDDGameListCache = lookup_bool(cfg, "devices.hdd_game_list_cache", gHDDGameListCache);
    gEnableWrite = lookup_bool(cfg, "devices.enable_write", gEnableWrite);
}

static void parse_paths(config_t *cfg)
{
    const char *path;
    if ((path = lookup_str(cfg, "paths.bdm_prefix", NULL)))
        strncpy(gBDMPrefix, path, sizeof(gBDMPrefix) - 1);

    if ((path = lookup_str(cfg, "paths.eth_prefix", NULL)))
        strncpy(gETHPrefix, path, sizeof(gETHPrefix) - 1);

    if ((path = lookup_str(cfg, "paths.mmce_prefix", NULL)))
        strncpy(gMMCEPrefix, path, sizeof(gMMCEPrefix) - 1);
}

static void parse_mmce(config_t *cfg)
{
    gMMCESlot = lookup_int(cfg, "mmce.slot", gMMCESlot);
    gMMCEIGRSlot = lookup_int(cfg, "mmce.igr_slot", gMMCEIGRSlot);
    gMMCEAckWaitCycles = lookup_int(cfg, "mmce.mmce_wait_cycles", gMMCEAckWaitCycles);
    gMMCEUseAlarms = lookup_bool(cfg, "mmce.use_alarms", gMMCEUseAlarms);
}

static void parse_debug(config_t *cfg)
{
    gEnableDebug = lookup_bool(cfg, "debug.enable_debug", gEnableDebug);
    gBDMDebug = lookup_bool(cfg, "debug.bdm_debug", gBDMDebug);
    gPS2Logo = lookup_bool(cfg, "debug.ps2_logo", gPS2Logo);
#ifdef __DEBUG
    gMMCEEnableGameID = lookup_bool(cfg, "debug.mmce_gameid", gMMCEEnableGameID);
#endif
}

static void build_display(config_setting_t *root)
{
    config_setting_t *group = add_group(root, "display");

    set_bool(group, "widescreen", gWideScreen);
    set_int(group, "vmode", gVMode);
    set_int(group, "x_offset", gXOff);
    set_int(group, "y_offset", gYOff);
    set_int(group, "overscan", gOverscan);
    set_int(group, "scroll_speed", gScrollSpeed);
    set_color(group, "bg_color", gDefaultBgColor);
    set_color(group, "text_color", gDefaultTextColor);
    set_color(group, "ui_text_color", gDefaultUITextColor);
    set_color(group, "sel_text_color", gDefaultSelTextColor);
    set_color(group, "plasma_blend_color", gDefaultPlasmaBlendColor);
}

static void build_ui(config_setting_t *root)
{
    config_setting_t *group = add_group(root, "ui");

    set_str(group, "theme", thmGetValue());
    set_str(group, "language", lngGetValue());
    set_bool(group, "swap_button", gSelectButton == KEY_CROSS);
    set_int(group, "x_sensitivity", gXSensitivity);
    set_int(group, "y_sensitivity", gYSensitivity);
    set_bool(group, "notifications", gEnableNotifications);
    set_bool(group, "disc_art", gDiscEnableArt);
}

static void build_audio(config_setting_t *root)
{
    config_setting_t *group = add_group(root, "audio");

    set_bool(group, "sfx", gEnableSFX);
    set_int(group, "sfx_volume", gSFXVolume);
    set_bool(group, "boot_sound", gEnableBootSND);
    set_int(group, "boot_volume", gBootSndVolume);
    set_bool(group, "bgm", gEnableBGM);
    set_int(group, "bgm_volume", gBGMVolume);
    set_str(group, "bgm_path", gDefaultBGMPath);
}

static void build_startup(config_setting_t *root)
{
    config_setting_t *group = add_group(root, "startup");

    set_int(group, "default_device", gDefaultDevice);
    set_bool(group, "auto_sort", gAutosort);
    set_bool(group, "auto_refresh", gAutoRefresh);
    set_bool(group, "remember_last", gRememberLastPlayed);
    set_bool(group, "autostart_last", gAutoStartLastPlayed);
    set_str(group, "exit_path", gExitPath);
    set_int(group, "bdm_start_mode", gBDMStartMode);
    set_int(group, "hdd_start_mode", gHDDStartMode);
    set_int(group, "eth_start_mode", gETHStartMode);
    set_int(group, "app_start_mode", gAPPStartMode);
    set_int(group, "fav_start_mode", gFAVStartMode);
    set_int(group, "mmce_start_mode", gMMCEStartMode);
}

static void build_devices(config_setting_t *root)
{
    config_setting_t *group = add_group(root, "devices");

    set_bool(group, "usb_enabled", gEnableUSB);
    set_bool(group, "ilink_enabled", gEnableILK);
    set_bool(group, "mx4sio_enabled", gEnableMX4SIO);
    set_bool(group, "bdm_hdd_enabled", gEnableBdmHDD);
    set_int(group, "bdm_cache", bdmCacheSize);
    set_int(group, "hdd_cache", hddCacheSize);
    set_int(group, "smb_cache", smbCacheSize);
    set_int(group, "hdd_spindown", gHDDSpindown);
    set_bool(group, "hdd_game_list_cache", gHDDGameListCache);
    set_bool(group, "enable_write", gEnableWrite);
}

static void build_paths(config_setting_t *root)
{
    config_setting_t *group = add_group(root, "paths");

    set_str(group, "bdm_prefix", gBDMPrefix);
    set_str(group, "eth_prefix", gETHPrefix);
    set_str(group, "mmce_prefix", gMMCEPrefix);
}

static void build_mmce(config_setting_t *root)
{
    config_setting_t *group = add_group(root, "mmce");

    set_int(group, "slot", gMMCESlot);
    set_int(group, "igr_slot", gMMCEIGRSlot);
    set_int(group, "mmce_wait_cycles", gMMCEAckWaitCycles);
    set_bool(group, "use_alarms", gMMCEUseAlarms);
}

static void build_debug(config_setting_t *root)
{
    config_setting_t *group = add_group(root, "debug");

    set_bool(group, "enable_debug", gEnableDebug);
    set_bool(group, "bdm_debug", gBDMDebug);
    set_bool(group, "ps2_logo", gPS2Logo);
#ifdef __DEBUG
    set_bool(group, "mmce_gameid", gMMCEEnableGameID);
#endif
}

int wOPLLoad(int *out_theme_id, int *out_lang_id)
{
    char dir[128];
    char path[256];

    if (!probe_config_path(dir, sizeof(dir), path, sizeof(path), 0))
        return 0;

    config_t cfg;
    config_init(&cfg);

    if (!config_read_file(&cfg, path)) {
        config_destroy(&cfg);
        LOG("CONFIG_WOPL: libconfig parse failed for '%s', attempting legacy migration\n", path);
        if (!migrate_legacy(path, out_theme_id, out_lang_id))
            return 0;

        strncpy(settings_config_dir, dir, sizeof(settings_config_dir) - 1);
        wOPLSave();
        LOG("CONFIG_WOPL: legacy config migrated to new format at '%s'\n", path);
        return 1;
    }

    parse_display(&cfg);
    parse_ui(&cfg, out_theme_id, out_lang_id);
    parse_audio(&cfg);
    parse_startup(&cfg);
    parse_devices(&cfg);
    parse_paths(&cfg);
    parse_mmce(&cfg);
    parse_debug(&cfg);

    config_destroy(&cfg);

    strncpy(settings_config_dir, dir, sizeof(settings_config_dir) - 1);
    LOG("CONFIG_WOPL: loaded from '%s'\n", path);
    return 1;
}

int wOPLSave(void)
{
    char dir[128];
    char path[256];

    if (settings_config_dir[0]) {
        strncpy(dir, settings_config_dir, sizeof(dir) - 1);
        dir[sizeof(dir) - 1] = '\0';
        snprintf(path, sizeof(path), "%s%s", dir, WOPL_FILENAME);
    } else {
        if (!probe_config_path(dir, sizeof(dir), path, sizeof(path), 1))
            return 0;
    }

    // Ensure MC directory exists before writing.
    if (!strncmp(dir, "mc", 2)) {
        char mc_dir[128];
        strncpy(mc_dir, dir, sizeof(mc_dir) - 1);
        mc_dir[sizeof(mc_dir) - 1] = '\0';
        size_t len = strlen(mc_dir);
        if (len > 0 && mc_dir[len - 1] == '/')
            mc_dir[len - 1] = '\0';
        if (!ensure_mc_dir(mc_dir)) {
            LOG("CONFIG_WOPL: failed to create MC dir '%s'\n", mc_dir);
            return 0;
        }
    }

    // Build tree from scratch each save
    config_t cfg;
    config_init(&cfg);
    config_setting_t *root = config_root_setting(&cfg);

    build_display(root);
    build_ui(root);
    build_audio(root);
    build_startup(root);
    build_devices(root);
    build_paths(root);
    build_mmce(root);
    build_debug(root);

    int ok = config_write_file(&cfg, path);
    config_destroy(&cfg);

    if (!ok) {
        LOG("CONFIG_WOPL: failed to write '%s'\n", path);
        return 0;
    }

    strncpy(settings_config_dir, dir, sizeof(settings_config_dir) - 1);
    LOG("CONFIG_WOPL: saved to '%s'\n", path);
    return 1;
}

const char *wOPLGetDir(void)
{
    return settings_config_dir[0] ? settings_config_dir : NULL;
}

// ---------------------------------------------------------------------------
// Legacy migration
//
// Called when libconfig fails to parse the file.. the file is in the old
// key=value format.. uses the existing config.c to read all
// values into globals, then wOPLSave() immediately rewrites them in
// the new libconfig format.. phase out eventually
// ---------------------------------------------------------------------------

static int migrate_legacy(const char *path, int *out_theme_id, int *out_lang_id)
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

    configClear(cfg);
    return 1;
}

