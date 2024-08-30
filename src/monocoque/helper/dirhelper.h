#ifndef _DIRHELPER_H
#define _DIRHELPER_H

#include <stdbool.h>

void create_dir(char* dir);
void create_xdg_dir(const char* dir);
char* create_user_dir(char* home_dir_str, const char* dirtype, const char* programpath);
char* gethome();
char* str2md5(const char* str, int length);
bool does_directory_exist(char* path);
void restrict_folders_to_cache(char* path, int cachesize);
void delete_dir(char* path);
bool does_file_exist(const char* file);

#endif
