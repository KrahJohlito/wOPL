#ifndef PATHSUPPORT_H
#define PATHSUPPORT_H

#include <stddef.h>

void pathSetLaunchPath(const char *path);
const char *pathGetLaunchPath(void);

void pathRegisterBDMDevice(int mass_index, const char *true_prefix);
void pathUnregisterBDMDevice(int mass_index);

int pathIsDevicePath(const char *path);
void pathNormaliseDir(char *dir, size_t dir_len);
int pathGetBootDir(char *dir_out, size_t dir_len);
int pathGetBootTrueDir(char *dir_out, size_t dir_len);
int pathResolveToTrue(char *out, size_t out_len, const char *path);
int pathResolveToRuntime(char *out, size_t out_len, const char *path);
int pathJoin(char *out, size_t out_len, const char *dir, const char *name);

#endif
