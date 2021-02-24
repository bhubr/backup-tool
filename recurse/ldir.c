#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#define FILE_MIN 1048576

int num_opened = 0;
int num_files = 0;
const char *accepted_exts[] = {"mp3","m4a","flac"};

char *get_filename_ext(const char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int search_array(const char **arr, char *str, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (!strncmp(arr[i], str, strlen(str))) return 1;
    }
    return 0;
}

void list_dirs(const char *name, int level)
{
    DIR *dir;
    struct dirent *entry;
    char *ext;


    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir))){
        closedir(dir);
        return;
    }

    do {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
            path[len] = 0;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            printf("      %*s[%s]\n", level*2, "", entry->d_name);
            list_dirs(path, level + 1);
        }
        else {
            ext = get_filename_ext( entry->d_name );
            if( search_array( accepted_exts, ext, 3 ) != -1 ) {
                num_files++;
                printf("%5d %*s- %s\n", num_files, level*2, "", entry->d_name);
            } 
        }
    } while ((entry = readdir(dir)));
    closedir(dir);
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        printf("Not enough arguments:\n  ldir <dir>\n\n");
        exit(-1);
    }

    list_dirs(argv[1], 0);
    return 0;
}

