#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h> // http://www.cplusplus.com/reference/ctime/difftime/

#define FILE_MIN 1048576

int num_opened = 0;
int num_files = 0;
int num_dirs = 0;
const char *accepted_exts[] = {"mp3","m4a","flac"};

char *get_filename_ext(const char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int search_array(const char **arr, char *str, int len) {
    int i;
    if (!strlen(str)) return 0;
    for (i = 0; i < len; i++) {
        if (!strncmp(arr[i], str, strlen(str))) return 1;
    }
    return 0;
}

void list_dirs(const char *name, int level, int max_level)
{
    DIR *dir;
    struct dirent *entry;
    char *ext;

    if ((max_level > -1) && (level == max_level)) return;

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
            num_dirs++;
            // printf("      %*s[%s]\n", level*2, "", entry->d_name);
            list_dirs(path, level + 1, max_level);
        }
        else {
            ext = get_filename_ext( entry->d_name );
            // if( search_array( accepted_exts, ext, 3 ) ) {
            num_files++;
            // printf("%5d %*s- %s\n", num_files, level*2, "", entry->d_name);
            // }
        }
    } while ((entry = readdir(dir)));
    closedir(dir);
}

int main(int argc, char **argv)
{
    time_t start;
    time_t end;
    double seconds;

    if(argc < 2) {
        printf("Not enough arguments:\n  ldir <dir>\n\n");
        exit(-1);
    }

    start = time(NULL);
    list_dirs(argv[1], 0, 2);
    end = time(NULL);

    seconds = difftime(end, start);
    printf("%d dirs scanned\n", num_dirs);
    printf("%d files found\n", num_files);
    printf("%f seconds elapsed\n", seconds);
    return 0;
}

