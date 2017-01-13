#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <jansson.h>
#include <sys/stat.h>
#include "../mp3/mp3_checksum.h"
#include "../http/post.h"
#include "run_md5.h"

#define FILE_MIN 1 // 048576

/**
 * Dump hex representation of string (for i.e. md5 hashes)
 */
void print_hex(unsigned char *s)
{
  while(*s) {
    printf("%02x", (unsigned char) (*s));
    s++;
  }
  printf("\n");
}

/**
 * Free a response object
 *   - Free all headers
 *   - Free JSON object
 *   - Free response itself
 */
void free_response(p_response response) {
    int i = 0;
    while(response->headers[i]) {
        // printf("# before header %d => %s\n", i, response->headers[i]);
        free(response->headers[i]);
        i++;
    }
    // printf("# headers strings done\n");
    free(response->headers);
    // printf("# headers array done\n");
    printf(" ====> response ptr before free: %p\n", response->raw_body);
    free(response->raw_body);
    printf("# raw body done\n");
    json_decref(response->json_body);
    printf("# json obj done\n");
    printf(" ====> response obj ptr before free: %p\n", response);
    free(response);
    printf("# response done\n");
}

/**
 * Get named header from response headers
 */
char *get_header(p_response response, char*key) {
    int i = 0, header_len;
    int key_len = strlen(key);
    while(response->headers[i]) {
        
        header_len = strlen(response->headers[i]);
        if (key_len > header_len + 1) return NULL;

        if(strncmp(response->headers[i], key, key_len) == 0) {
            printf("get_header %s %d - print: %s\n", key, i, response->headers[i]);
            return response->headers[i] + key_len + 2;
        }
        i++;
    }
    return NULL;
}

int init_session(char* email, char* password, char *mount_point, char** headers)
{
    p_response req_result;
    unsigned char *post_data;
    json_t *success_j;
    bool success;
    char *cookie_value;

    post_data = malloc(200);
    sprintf(post_data, "email=%s&password=%s&mount_point=%s", email, password, mount_point);
    req_result = send_request("192.168.1.71", 8000, "POST", "/login", post_data, headers);

    if(req_result->json_body == NULL) {
        printf("REQUEST FAILED: non-JSON response\n");
        exit(-1);
    }
    success_j = json_object_get(req_result->json_body, "success");
    success = json_boolean_value(success_j);
    if (! success) {
        json_t *error_j = json_object_get(req_result->json_body, "error");
        printf("Encountered error: %s\n", json_string_value(error_j));
        exit(0);
    }

    cookie_value = get_header(req_result, "Set-Cookie");
    headers[1] = malloc(9 + strlen(cookie_value));
    sprintf(headers[1], "Cookie: %s", cookie_value);
    headers[2] = 0;
    free_response(req_result);
    free(post_data);
    return 0;
}

void listdir(const char *name, int level, int parent_id, char** headers)
{
    DIR *dir;
    struct dirent *entry;
    unsigned char *md5, *md5_ptr, *md5_hex, post_data[2048];
    p_response req_result;
    json_t *value;
    int i, filesize;
    char *cookie_value, *set_cookie;
    struct stat st;

    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return;

    if( level == 0) {
printf("--- level 0 BEGIN ---\n");
        // post_data = malloc(150 + strlen(name));
        sprintf(post_data, "parent_id=%d&type=D&name=%s", parent_id, (unsigned char*)name);
        printf("ROOT %s\n", name);
        req_result = send_request("192.168.1.71", 8000, "POST", "/files", post_data, headers);
        value = json_object_get(req_result->json_body, "id");
        parent_id = json_integer_value(value);
        printf("---- Root dir %s has id %d ----\n\n", name, parent_id);
printf("--- level 0 BEFORE FREE ---\n");
        free_response(req_result);
        // free(post_data);
printf("--- level 0 END ---\n");
    }

    do {
        char path[1024];
        printf("entry: %s type: %d\n", entry->d_name, entry->d_type);
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        if (entry->d_type == DT_DIR) {
printf("--- DIR BEGIN ---\n");
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            printf("%*s[%s]\n", level*2, "", entry->d_name);
            // post_data = malloc(150 + strlen(entry->d_name));
            printf("DIR %s\n", entry->d_name);
            sprintf(post_data, "parent_id=%d&type=D&name=%s", parent_id, entry->d_name);
            printf("DIR %s post_data %s\n", entry->d_name, post_data);
            req_result = send_request("192.168.1.71", 8000, "POST", "/files", post_data, headers);
            printf("response obj ptr, JSON body ptr: %p %p\n", req_result, req_result->json_body);
            value = json_object_get(req_result->json_body, "id");

            printf("GOT ID %lld\n", json_integer_value(value) );
            // json_decref(req_result->json_body);
            // printf("HEADER DUMP for Set-Cookie => [%s]\n", get_header(req_result, "Set-Cookie"));
            // printf("HEADER DUMP for Content-Type => [%s]\n", get_header(req_result, "Content-Type"));
printf("--- DIR BEFORE FREE ---\n");
            free_response(req_result);
printf("#1\n");
            // free(post_data);
printf("--- DIR END ---\n");
            listdir(path, level + 1, parent_id, headers);
        }
        else {
            stat(path, &st);
            filesize = st.st_size;
            if(filesize < FILE_MIN) {
                // printf( "%s filesize %d < %d, skip\n", path, filesize, FILE_MIN );
                continue;
            }

            int entry_len = strlen(entry->d_name);

            if (strcmp(entry->d_name + entry_len - 4 , ".mp3") != 0) {
printf("--- REG BEGIN ---\n");
                
                md5 = run_md5(path);
                printf("run md5 on entry: %s type: %d  => md5: %s\n", entry->d_name, entry->d_type, md5);
                // post_data = malloc(150 + strlen(entry->d_name));
                sprintf(post_data, "parent_id=%d&type=F&name=%s&md5=%s", parent_id, entry->d_name, md5);
                printf("post_data %s\n", post_data);
                // for(i=0; i<16;i++){
                //     sprintf(post_data + strlen(post_data), "%02x", md5[i]);
                // }
                printf("REG %s\n", entry->d_name);
                req_result = send_request("192.168.1.71", 8000, "POST", "/files", post_data, headers);
printf("--- REG BEFORE FREE ---\n");
                free(md5);
printf("#1\n");
                free_response(req_result);
printf("#2\n");
                // free(post_data);
printf("--- REG END ---\n");
            }
            else {
printf("--- MP3 BEGIN ---\n");
                md5 = mp3_checksum(path);
printf("--- MP3 md5: %s ---\n", md5);
                // post_data = malloc(150 + strlen(entry->d_name));
                sprintf(post_data, "parent_id=%d&type=F&name=%s&md5=", parent_id, entry->d_name);
printf("--- MP3 post data #1: %s ---\n", post_data);
                for(i=0; i<16;i++){
                    sprintf(post_data + strlen(post_data), "%02x", md5[i]);
                }
printf("--- MP3 post data #2: %s ---\n", post_data);
                printf("MP3 %s BEFORE REQ\n", entry->d_name);
                req_result = send_request("192.168.1.71", 8000, "POST", "/files", post_data, headers);
printf("--- MP3 BEFORE FREE ---\n");
                free(md5);
printf("#1\n");
                free_response(req_result);
printf("#2\n");
                // free(post_data);
printf("--- MP3 END ---\n");
            }
            printf("%*s- %s\n", level*2, "", entry->d_name);
        }
    } while ((entry = readdir(dir)));
    closedir(dir);
}

int main(int argc, char **argv)
{
    // We send data as urlencoded
    char *content_type = "Content-Type: application/x-www-form-urlencoded";

    // Initialize headers array (leave a slot for Cookie header)
    char *headers[3];

    // Allocate and initialize content header
    headers[0] = malloc(strlen(content_type) + 1);
    strcpy(headers[0], content_type);
    headers[1] = 0;

    if(argc < 5) {
        printf("Not enough arguments:\n  listdir <dir> <email> <pass> <mount point>");
        exit(-1);
    }
    init_session(argv[2], argv[3], argv[4], headers);

    listdir(argv[1], 0, 0, headers);
    free(headers[0]);
    free(headers[1]);
    return 0;
}
