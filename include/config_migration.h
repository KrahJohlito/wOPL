#ifndef CONFIG_MIGRATION_H
#define CONFIG_MIGRATION_H

#include "include/iosupport.h"

// Each returns 1 on success.. 0 if not found or failed
int cfgMigrateLegacyOPL(const char *path, int *out_theme_id, int *out_lang_id);
int cfgMigrateLegacyNet(const char *path);
int cfgMigrateLegacyGlobalGame(const char *path);
int cfgMigrateLegacyPerGame(const char *path, per_game_cfg_t *cfg);
int cfgMigrateLegacyGameInfo(const char *path, game_info_t *gi);

// Returns 1 if anything was loaded from TAR.. 0 if nothing found
// gi and/or pgcfg may be NULL if caller only needs one of them
int cfgMigrateTARGameCfg(const char *startup, game_info_t *gi, per_game_cfg_t *pgcfg);

#endif
