#ifndef RAMFS_H
#define RAMFS_H

#include <stdint.h>
#include <stdbool.h>

typedef struct File
{
    char name[32];
    char type;
    char content[1024];
    int size;
    struct File *parent;
    struct File *child;
    struct File *next;
} File;

void fs_init(void);
void fs_mkdir(const char *name);
void fs_mkfile(const char *name);
void fs_cd(const char *path);
void fs_ls(void);
void fs_write(const char *filename, const char *content);
void fs_cat(const char *filename);
void fs_pwd(void);
void fs_tparse(const char *filename);

void cmd_dir(const char *args);
void cmd_fs(const char *args);
void cmd_cd(const char *args);
void cmd_write(const char *args);
void cmd_ls(const char *args);
void cmd_cat(const char *args);
void cmd_pwd(void);
void cmd_tparse(const char *args);

#endif