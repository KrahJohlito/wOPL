#ifndef CONFIG_WOPL_H
#define CONFIG_WOPL_H

#include "include/iosupport.h"

extern int gBDMFramesDelay;
extern int gETHFramesDelay;
extern int gHDDFramesDelay;
extern int gMMCEFramesDelay;
extern int gAPPFramesDelay;
extern int gFAVFramesDelay;

void dnas_to_binary(const char *dnas, char *out, int out_size);

const char *wOPLGetDir(void);
const char *wOPLGetThemeName(void);
const char *wOPLGetLanguageName(void);

int wOPLLoad(int *out_theme_id, int *out_lang_id);
int wOPLSave(void);

int wOPLNetLoad(void);
int wOPLNetSave(void);

int wOPLLastLoad(void);
int wOPLLastSave(const char *startup);
const char *wOPLLastGet(void);

int wOPLGlobalGameLoad(void);
int wOPLGlobalGameSave(void);
int wOPLPerGameLoad(const char *path, per_game_cfg_t *cfg);
int wOPLPerGameSave(const char *path, const per_game_cfg_t *cfg);

int wOPLGameInfoLoad(const char *path, game_info_t *gi);
int wOPLGameInfoSave(const char *path, const game_info_t *gi);

extern char gParentalLockPassword[256];
extern char *gBaseMCDir;

void loadConfig();
void configApply(int themeID, int langID, int skipDeviceRefresh);
int configLoad(int types);
int configSave(int types, int showUI);

#endif
