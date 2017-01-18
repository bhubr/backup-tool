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
#include "post.h"
#include "run_md5.h"

#define FILE_MIN 1048576


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
    free(response->raw_body);
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
            // printf("get_header %s %d - print: %s\n", key, i, response->headers[i]);
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
    unsigned char *md5, *mp3_md5, *md5_ptr, *md5_hex, post_data[2048];
    p_response req_result;
    json_t *file_j, *id_j;
    int i, filesize;
    char *cookie_value, *set_cookie;
    struct stat st;
    int new_parent_id;

    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return;

    if( level == 0) {
        sprintf(post_data, "parent_id=%d&type=D&name=%s", parent_id, (unsigned char*)name);
        // printf("ROOT %s\n", name);
        req_result = send_request("192.168.1.71", 8000, "POST", "/files", post_data, headers);
        file_j = json_object_get(req_result->json_body, "file");
        id_j = json_object_get(file_j, "id");
        parent_id = json_integer_value(id_j);
        printf("---- Root dir %s has id %d ----\n\n", name, parent_id);
        free_response(req_result);
    }

    do {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);

        // IS DIRECTORY
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            printf("%*s[%s]\n", level*2, "", entry->d_name);
            sprintf(post_data, "parent_id=%d&type=D&name=%s", parent_id, entry->d_name);
            req_result = send_request("192.168.1.71", 8000, "POST", "/files", post_data, headers);
            file_j = json_object_get(req_result->json_body, "file");
            id_j = json_object_get(file_j, "id");
            new_parent_id = json_integer_value(id_j);
            printf("---- dir %s has id %d ----\n\n", name, new_parent_id);

            free_response(req_result);
            // printf("cookie after D [%s]\n", headers[1]);
            listdir(path, level + 1, new_parent_id, headers);
        }
        // IS REGULAR FILE
        else {

            // IGNORE SMALL FILES
            stat(path, &st);
            filesize = st.st_size;
            if(filesize < FILE_MIN) {
                continue;
            }

            md5 = run_md5(path);
            sprintf(post_data, "parent_id=%d&type=F&name=%s&md5=%s", parent_id, entry->d_name, md5);

            // For mp3 files, add md5 sum of audio part, stripped of ID3v* tags
            int entry_len = strlen(entry->d_name);
            if (strcmp(entry->d_name + entry_len - 4 , ".mp3") == 0) {
                mp3_md5 = mp3_checksum(path);
                strcat(post_data, "&mp3_md5=");
                for(i=0; i<16;i++){
                    sprintf(post_data + strlen(post_data), "%02x", mp3_md5[i]);
                }
            }

            // fire request
            req_result = send_request("192.168.1.71", 8000, "POST", "/files", post_data, headers);
            free(md5);
            free_response(req_result);
            // printf("cookie after F [%s]\n", headers[1]);
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
