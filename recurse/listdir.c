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

#define FILE_MIN 1048576

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
        free(response->headers[i]);
        i++;
    }
    free(response->headers);
    json_decref(response->json_body);
    free(response);
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

int init_session(char* email, char* password, char** headers)
{
    p_response req_result;
    unsigned char *post_data;
    json_t *success_j;
    bool success;
    char *cookie_value;

    post_data = malloc(200);
    sprintf(post_data, "email=%s&password=%s", email, password);
    // printf("ROOT %s\n", name);
    req_result = send_request("localhost", 8000, "POST", "/login", post_data, headers);

    if(req_result->json_body == NULL) {
        printf("REQUEST FAILED: non-JSON response\n");
        exit(-1);
    }
    success_j = json_object_get(req_result->json_body, "success");
    success = json_boolean_value(success_j);
    printf("Status: %s\n", success ? "OK":"Error");
    if (! success) {
        json_t *error_j = json_object_get(req_result->json_body, "error");
        printf("Encountered error: %s\n", json_string_value(error_j));
        exit(0);
    }
    // printf("---- Root dir %s has id %d ----\n\n", name, parent_id);
    // // json_decref(req_result->json_body);


    cookie_value = get_header(req_result, "Set-Cookie");
    printf("Cookie value: [%s]\n", cookie_value);
    print_hex(cookie_value);
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
    unsigned char *md5, *md5_ptr, *md5_hex, *post_data;
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

        post_data = malloc(150 + strlen(name));
        sprintf(post_data, "parent_id=%d&type=D&name=%s", parent_id, (unsigned char*)name);
        printf("ROOT %s\n", name);
        req_result = send_request("localhost", 8000, "POST", "/files", post_data, headers);
        value = json_object_get(req_result->json_body, "id");
        parent_id = json_integer_value(value);
        printf("---- Root dir %s has id %d ----\n\n", name, parent_id);

        free_response(req_result);
        free(post_data);
    }

    do {
        char path[1024];
        // printf("entry: %s type: %d\n", entry->d_name, entry->d_type);
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            printf("%*s[%s]\n", level*2, "", entry->d_name);
            post_data = malloc(150 + strlen(entry->d_name));
            printf("DIR %s\n", entry->d_name);
            sprintf(post_data, "parent_id=%d&type=D&name=%s", parent_id, entry->d_name);
            req_result = send_request("localhost", 8000, "POST", "/files", post_data, headers);
            // json_decref(req_result->json_body);
            // printf("HEADER DUMP for Set-Cookie => [%s]\n", get_header(req_result, "Set-Cookie"));
            // printf("HEADER DUMP for Content-Type => [%s]\n", get_header(req_result, "Content-Type"));
            free_response(req_result);
            free(post_data);

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
                // printf("run md5 on entry: %s type: %d\n", entry->d_name, entry->d_type);
                md5 = run_md5(path);
                post_data = malloc(150 + strlen(entry->d_name));
                sprintf(post_data, "parent_id=%d&type=F&name=%s&md5=", parent_id, entry->d_name);
                for(i=0; i<16;i++){
                    sprintf(post_data + strlen(post_data), "%02x", md5[i]);
                }
                printf("REG %s\n", entry->d_name);
                req_result = send_request("localhost", 8000, "POST", "/files", post_data, headers);
                free(md5);
                // json_decref(req_result->json_body);
                free_response(req_result);
                free(post_data);
            }
            else {
                md5 = mp3_checksum(path);
                post_data = malloc(150 + strlen(entry->d_name));
                sprintf(post_data, "parent_id=%d&type=F&name=%s&md5=", parent_id, entry->d_name);
                for(i=0; i<16;i++){
                    sprintf(post_data + strlen(post_data), "%02x", md5[i]);
                }
                printf("MP3 %s\n", entry->d_name);
                req_result = send_request("localhost", 8000, "POST", "/files", post_data, headers);
                free(md5);
                free_response(req_result);
                free(post_data);
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

    if(argc < 4) {
        printf("Not enough arguments:\n  listdir <dir> <email> <pass>");
        exit(-1);
    }
    init_session(argv[2], argv[3], headers);

    listdir(argv[1], 0, 0, headers);
    free(headers[0]);
    free(headers[1]);
    return 0;
}
