#ifndef CONFIG_WOPL_H
#define CONFIG_WOPL_H

void dnas_to_binary(const char *dnas, char *out, int out_size);

int wOPLLoad(int *out_theme_id, int *out_lang_id);
int wOPLSave(void);
const char *wOPLGetDir(void);

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

#endif
