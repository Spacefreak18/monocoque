#ifndef _DIRHELPER_H
#define _DIRHELPER_H

#include <stdbool.h>

char* gethome();
char* str2md5(const char* str, int length);
bool does_directory_exist(char* path, char* dirname);
void restrict_folders_to_cache(char* path, int cachesize);
void delete_dir(char* path);

#endif
