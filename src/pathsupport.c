#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "include/pathsupport.h"

#define PATH_MAX_BDM_DEVICES 16

typedef struct
{
    int valid;
    int massIndex;
    char runtimePrefix[16];
    char truePrefix[16];
} path_bdm_device_t;

static path_bdm_device_t bdmDevices[PATH_MAX_BDM_DEVICES];
static char launchPath[256];

static void copy_str(char *dst, const char *src, size_t size)
{
    if (!size)
        return;

    if (!src)
        src = "";

    strncpy(dst, src, size - 1);
    dst[size - 1] = '\0';
}

static int path_starts_with_device(const char *path, const char *device)
{
    const char *suffix;
    size_t len;

    if (!path || !device)
        return 0;

    len = strlen(device);

    if (strncmp(path, device, len))
        return 0;

    suffix = path + len;

    while (*suffix >= '0' && *suffix <= '9')
        suffix++;

    return *suffix == ':';
}

static int path_replace_prefix(char *out, size_t out_len, const char *prefix, const char *tail)
{
    int len;

    if (!out || !out_len || !prefix || !tail)
        return 0;

    len = snprintf(out, out_len, "%s%s", prefix, tail);

    if (len < 0 || len >= out_len) {
        out[0] = '\0';
        return 0;
    }

    return 1;
}

static int path_get_mass_index(const char *path, int *index, const char **tail)
{
    const char *p;
    int value;

    if (!path || strncmp(path, "mass", 4))
        return 0;

    p = path + 4;
    value = 0;

    if (*p < '0' || *p > '9')
        return 0;

    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
    }

    if (*p != ':')
        return 0;

    if (index)
        *index = value;

    if (tail)
        *tail = p + 1;

    return 1;
}

static path_bdm_device_t *path_find_bdm_by_mass_index(int mass_index)
{
    int i;

    for (i = 0; i < PATH_MAX_BDM_DEVICES; i++) {
        if (bdmDevices[i].valid && bdmDevices[i].massIndex == mass_index)
            return &bdmDevices[i];
    }

    return NULL;
}

static path_bdm_device_t *path_find_bdm_by_true_prefix(const char *path)
{
    int i;
    size_t len;

    if (!path)
        return NULL;

    for (i = 0; i < PATH_MAX_BDM_DEVICES; i++) {
        if (!bdmDevices[i].valid)
            continue;

        len = strlen(bdmDevices[i].truePrefix);

        if (len > 0 && !strncmp(path, bdmDevices[i].truePrefix, len))
            return &bdmDevices[i];
    }

    return NULL;
}

void pathSetLaunchPath(const char *path)
{
    copy_str(launchPath, path, sizeof(launchPath));
}

const char *pathGetLaunchPath(void)
{
    return launchPath[0] ? launchPath : NULL;
}

void pathRegisterBDMDevice(int mass_index, const char *true_prefix)
{
    path_bdm_device_t *device;

    if (mass_index < 0 || mass_index >= PATH_MAX_BDM_DEVICES || !true_prefix || !true_prefix[0])
        return;

    device = &bdmDevices[mass_index];

    device->valid = 1;
    device->massIndex = mass_index;
    snprintf(device->runtimePrefix, sizeof(device->runtimePrefix), "mass%d:", mass_index);
    copy_str(device->truePrefix, true_prefix, sizeof(device->truePrefix));
}

void pathUnregisterBDMDevice(int mass_index)
{
    if (mass_index < 0 || mass_index >= PATH_MAX_BDM_DEVICES)
        return;

    memset(&bdmDevices[mass_index], 0, sizeof(bdmDevices[mass_index]));
}

int pathIsDevicePath(const char *path)
{
    static const char *devices[] = {
        "mc",
        "mass", // legacy cwd/runtime alias only
        "usb",
        "mx4sio",
        "ilink",
        "ata",
        "hdd",
        "host",
        "mmce",
        NULL};

    int i;

    if (!path || !path[0])
        return 0;

    for (i = 0; devices[i] != NULL; i++) {
        if (path_starts_with_device(path, devices[i]))
            return 1;
    }

    return 0;
}

void pathNormaliseDir(char *dir, size_t dir_len)
{
    size_t len;

    if (!dir_len)
        return;

    dir[dir_len - 1] = '\0';
    len = strlen(dir);

    if (len > 0 && dir[len - 1] != '/') {
        if (len + 1 < dir_len) {
            dir[len] = '/';
            dir[len + 1] = '\0';
        }
    }
}

static int path_get_dirname(const char *path, char *dir_out, size_t dir_len)
{
    const char *slash;

    if (!path || !path[0] || !dir_len)
        return 0;

    if (!pathIsDevicePath(path))
        return 0;

    slash = strrchr(path, '/');

    if (!slash) {
        copy_str(dir_out, path, dir_len);
        pathNormaliseDir(dir_out, dir_len);
        return 1;
    }

    if ((size_t)(slash - path + 1) >= dir_len)
        return 0;

    memcpy(dir_out, path, slash - path + 1);
    dir_out[slash - path + 1] = '\0';

    return 1;
}

int pathGetBootDir(char *dir_out, size_t dir_len)
{
    char pwd[128];

    if (!dir_out || !dir_len)
        return 0;

    if (path_get_dirname(launchPath, dir_out, dir_len))
        return 1;

    pwd[0] = '\0';

    if (getcwd(pwd, sizeof(pwd)) == NULL)
        return 0;

    if (!pathIsDevicePath(pwd))
        return 0;

    copy_str(dir_out, pwd, dir_len);
    pathNormaliseDir(dir_out, dir_len);

    return 1;
}

int pathGetBootTrueDir(char *dir_out, size_t dir_len)
{
    char dir[256];

    if (!pathGetBootDir(dir, sizeof(dir)))
        return 0;

    return pathResolveToTrue(dir_out, dir_len, dir);
}

int pathResolveToTrue(char *out, size_t out_len, const char *path)
{
    path_bdm_device_t *device;
    const char *tail;
    int mass_index;

    if (!out || !out_len || !path)
        return 0;

    if (path_get_mass_index(path, &mass_index, &tail)) {
        device = path_find_bdm_by_mass_index(mass_index);

        if (device)
            return path_replace_prefix(out, out_len, device->truePrefix, tail);
    }

    copy_str(out, path, out_len);

    return 1;
}

int pathResolveToRuntime(char *out, size_t out_len, const char *path)
{
    path_bdm_device_t *device;
    const char *tail;
    size_t len;

    if (!out || !out_len || !path)
        return 0;

    device = path_find_bdm_by_true_prefix(path);

    if (device) {
        len = strlen(device->truePrefix);
        tail = path + len;

        return path_replace_prefix(out, out_len, device->runtimePrefix, tail);
    }

    copy_str(out, path, out_len);

    return 1;
}

int pathJoin(char *out, size_t out_len, const char *dir, const char *name)
{
    int len;

    if (!out || !out_len || !dir || !dir[0] || !name)
        return 0;

    len = snprintf(out, out_len, "%s%s", dir, name);

    if (len < 0 || len >= out_len) {
        out[0] = '\0';
        return 0;
    }

    return 1;
}
