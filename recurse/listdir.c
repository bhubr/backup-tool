#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../mp3/mp3_checksum.h"
#include "../http/post.h"
#include "run_md5.h"

void print_hex(unsigned char *s)
{
  while(*s) {
    printf("%02x", (unsigned char) (*s));
    s++;
  }
    
  printf("\n");
}

void listdir(const char *name, int level)
{
    DIR *dir;
    struct dirent *entry;
    unsigned char *md5, *md5_ptr, *md5_hex, *req_result, *post_data;
    char *headers[2];
    int i;
    char *content_type = "Content-Type: application/x-www-form-urlencoded";

    headers[0] = malloc(strlen(content_type) + 1);
    strcpy(headers[0], content_type);
    headers[1] = 0;


    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return;

    do {
        char path[1024];
        int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
        path[len] = 0;
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            printf("%*s[%s]\n", level*2, "", entry->d_name);
            listdir(path, level + 1);
        }
        else {
            int entry_len = strlen(entry->d_name);

            if (strcmp(entry->d_name + entry_len - 4 , ".mp3") != 0) {
                run_md5(path);
            }
            else {
                md5 = mp3_checksum(path);
                post_data = malloc(43 + strlen(path));
                sprintf(post_data, "path=%s&md5=", path);
                for(i=0; i<16;i++){
                    sprintf(post_data + strlen(post_data), "%02x", md5[i]);
                }
                req_result = send_request("192.168.1.49", 8000, "POST", "/", post_data, headers);
                free(md5);
                free(req_result);
            }
            printf("%*s- %s\n", level*2, "", entry->d_name);
        }
    } while ((entry = readdir(dir)));
    closedir(dir);
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        printf("Not enough arguments");
        exit(-1);
    }
    listdir(argv[1], 0);
    return 0;
}
