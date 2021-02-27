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

// added count of dirs per level
// now handle callback
void list_dirs(const char *name, int level, int *num_per_level, void (*cb)(int, int), int max_level)
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
            num_per_level[level]++;
            printf("      %*s[%s]\n", level*2, "", entry->d_name);

            list_dirs(path, level + 1, num_per_level, cb, max_level);
        }
        else {
            ext = get_filename_ext( entry->d_name );
            // if( search_array( accepted_exts, ext, 3 ) ) {
            num_files++;
            // printf("%5d %*s- %s\n", num_files, level*2, "", entry->d_name);
            // }
        }
    } while ((entry = readdir(dir)));
    cb(level, num_dirs);
    closedir(dir);
}

void fun(int a, int b) { printf("Fun %d %d\n\n", a, b); }

int main(int argc, char **argv)
{
    time_t start;
    time_t end;
    double seconds;
    int MAX = 3;
    int *num_per_level;

    num_per_level = malloc(MAX);
    for (int i = 0; i < MAX; i++) num_per_level[0] = 0;

    if(argc < 2) {
        printf("Not enough arguments:\n  ldir <dir>\n\n");
        exit(-1);
    }

    start = time(NULL);
    list_dirs(argv[1], 0, num_per_level, &fun, MAX);
    end = time(NULL);

    seconds = difftime(end, start);
    printf("%d dirs scanned\n", num_dirs);
    for (int i = 0; i < MAX; i++) printf("\t%2d: %d\n", i, num_per_level[i]);
    printf("%d files found\n", num_files);
    printf("%f seconds elapsed\n", seconds);

    free(num_per_level);
    return 0;
}

