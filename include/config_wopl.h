/*
  TODO: conf_game.cfg, conf_network.cfg, etc.)
*/

#ifndef CONFIG_WOPL_H
#define CONFIG_WOPL_H

int wOPLLoad(int *out_theme_id, int *out_lang_id);
int wOPLSave(void);
const char *wOPLGetDir(void);

#endif
