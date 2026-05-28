#include "include/config_wopl_migrate.h"
#include "include/config.h"
#include "include/themes.h"
#include "include/lang.h"
#include "include/pad.h"
#include "include/sound.h"
#include "include/system.h"
#include "include/ioman.h"

#ifdef __DEBUG
#include "include/debug.h"
#endif

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
