#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include <sys/stat.h>
#include "../mp3/mp3_checksum.h"
#include "../http/post.h"
#include "run_md5.h"

#define FILE_MIN 1048576

void print_hex(unsigned char *s)
{
  while(*s) {
    printf("%02x", (unsigned char) (*s));
    s++;
  }
    
  printf("\n");
}

void listdir(const char *name, int level, int parent_id)
{
    DIR *dir;
    struct dirent *entry;
    unsigned char *md5, *md5_ptr, *md5_hex, *post_data;
    json_t *req_result, *value;
    char *headers[2];
    int i, filesize;
    char *content_type = "Content-Type: application/x-www-form-urlencoded";
    struct stat st;

    headers[0] = malloc(strlen(content_type) + 1);
    strcpy(headers[0], content_type);
    headers[1] = 0;


    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return;

    post_data = malloc(150 + strlen(name));
    sprintf(post_data, "parent_id=%d&type=D&name=%s", parent_id, (unsigned char*)name);
    req_result = send_request("localhost", 8000, "POST", "/files", post_data, headers);
    value = json_object_get(req_result, "id");
    parent_id = json_integer_value(value);
    // printf("---- Root dir %s has id %d ----\n\n", name, parent_id);
    json_decref(req_result);
    free(post_data);


    do {
        char path[1024];
        // printf("entry: %s type: %d\n", entry->d_name, entry->d_type);
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            printf("%*s[%s]\n", level*2, "", entry->d_name);
            post_data = malloc(150 + strlen(entry->d_name));
            // printf("DIR %s", entry->d_name);
            sprintf(post_data, "parent_id=%d&type=D&name=%s", parent_id, entry->d_name);
            req_result = send_request("localhost", 8000, "POST", "/files", post_data, headers);
            json_decref(req_result);
            free(post_data);

            listdir(path, level + 1, parent_id);
        }
        else {
            stat(path, &st);
            filesize = st.st_size;
            if(filesize < FILE_MIN) {
                printf( "%s filesize %d < %d, skip\n", path, filesize, FILE_MIN );
                continue;
            }

            int entry_len = strlen(entry->d_name);

            if (strcmp(entry->d_name + entry_len - 4 , ".mp3") != 0) {
                // printf("run md5 on entry: %s type: %d\n", entry->d_name, entry->d_type);
                md5 = run_md5(path);
                post_data = malloc(150 + strlen(entry->d_name));
                sprintf(post_data, "parent_id=%d&type=F&name=%s&md5=", parent_id, entry->d_name);
                for(i=0; i<16;i++){
                    sprintf(post_data + strlen(post_data), "%02x", md5[i]);
                }
                req_result = send_request("localhost", 8000, "POST", "/files", post_data, headers);
                free(md5);
                json_decref(req_result);
                free(post_data);
            }
            else {
                md5 = mp3_checksum(path);
                post_data = malloc(150 + strlen(entry->d_name));
                sprintf(post_data, "parent_id=%d&type=F&name=%s&md5=", parent_id, entry->d_name);
                for(i=0; i<16;i++){
                    sprintf(post_data + strlen(post_data), "%02x", md5[i]);
                }
                req_result = send_request("localhost", 8000, "POST", "/files", post_data, headers);
                free(md5);
                json_decref(req_result);
                free(post_data);
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
    listdir(argv[1], 0, 0);
    return 0;
}
