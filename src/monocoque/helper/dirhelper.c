#include "dirhelper.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>

#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <string.h>

char* gethome()
{
    char* homedir = getenv("HOME");
    return homedir;

    if (homedir != NULL)
    {
        printf("Home dir in enviroment");
        printf("%s\n", homedir);
    }

    uid_t uid = getuid();
    struct passwd* pw = getpwuid(uid);

    if (pw == NULL)
    {
        printf("Failed\n");
        exit(EXIT_FAILURE);
    }

    return pw->pw_dir;
}

time_t get_file_creation_time(char* path)
{
    struct stat attr;
    stat(path, &attr);
    return attr.st_mtime;
}

void delete_dir(char* path)
{

    struct dirent* de;
    DIR* dr = opendir(path);

    if (dr == NULL)
    {
        printf("Could not open current directory");
    }

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    while ((de = readdir(dr)) != NULL)
    {
        char* fullpath = ( char* ) malloc(1 + strlen(path) + strlen("/") + strlen(de->d_name));
        strcpy(fullpath, path);
        strcat(fullpath, "/");
        strcat(fullpath, de->d_name);
        unlink(fullpath);
        free(fullpath);
    }
    closedir(dr);
    rmdir(path);

}

void delete_oldest_dir(char* path)
{
    char* oldestdir = path;

    struct dirent* de;
    DIR* dr = opendir(path);

    if (dr == NULL)
    {
        printf("Could not open current directory");
    }

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    char filename_qfd[100] ;
    char* deletepath = NULL;
    time_t tempoldest = 0;
    while ((de = readdir(dr)) != NULL)
    {
        struct stat stbuf;

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
        {
            continue;
        }

        char* fullpath = ( char* ) malloc(1 + strlen(path) + strlen(de->d_name));
        strcpy(fullpath, path);
        strcat(fullpath, de->d_name);

        stat(fullpath, &stbuf);
        if (S_ISDIR(stbuf.st_mode))
        {
            strcpy(fullpath, path);
            strcat(fullpath, de->d_name);
            if (tempoldest == 0)
            {
                tempoldest = get_file_creation_time(fullpath);
                free(deletepath);
                deletepath = strdup(fullpath);
            }
            else
            {
                time_t t = get_file_creation_time(fullpath);
                double diff = tempoldest - t;
                if (diff > 0)
                {
                    tempoldest = t;
                    free(deletepath);
                    deletepath = strdup(fullpath);
                }
            }

        }

        free(fullpath);
    }
    closedir(dr);
    delete_dir(deletepath);
    free(deletepath);
}

void restrict_folders_to_cache(char* path, int cachesize)
{
    int numfolders = 0;

    struct dirent* de;
    DIR* dr = opendir(path);

    if (dr == NULL)
    {
        printf("Could not open current directory");
    }

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    while ((de = readdir(dr)) != NULL)
    {

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
        {
            continue;
        }

        char* fullpath = ( char* ) malloc(1 + strlen(path) + strlen(de->d_name));
        strcpy(fullpath, path);
        strcat(fullpath, de->d_name);
        strcat(fullpath, "/");

        struct stat stbuf;
        stat(fullpath,&stbuf);

        if (S_ISDIR(stbuf.st_mode))
        {
            numfolders++;
        }
        free(fullpath);
    }

    while (numfolders >= cachesize)
    {
        delete_oldest_dir(path);
        numfolders--;
    }
    closedir(dr);

}

bool does_directory_exist(char* path, char* dirname)
{
    struct dirent* de;
    DIR* dr = opendir(path);

    if (dr == NULL)
    {
        printf("Could not open current directory");
        return false;
    }

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    bool answer = false;
    while ((de = readdir(dr)) != NULL)
    {
        if (strcmp(dirname,de->d_name) == 0)
        {
            answer = true;
        }
    }

    closedir(dr);
    return answer;
}

