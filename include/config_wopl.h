#ifndef CONFIG_WOPL_H
#define CONFIG_WOPL_H

int wOPLLoad(int *out_theme_id, int *out_lang_id);
int wOPLSave(void);
const char *wOPLGetDir(void);

int wOPLNetLoad(void);
int wOPLNetSave(void);

int wOPLLastLoad(void);
int wOPLLastSave(const char *startup);
const char *wOPLLastGet(void);

#endif
