#include <curl/curl.h>
#include <dirent.h>
#include <jansson.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h> // http://www.cplusplus.com/reference/ctime/difftime/
#include <unistd.h>

#define FILE_MIN 1048576

int num_opened = 0;
int num_files = 0;
int num_dirs = 0;
const char *accepted_exts[] = {"mp3","m4a","flac"};
const char *ignored[] = {"node_modules", ".pnpm", "AppData"};
json_t *all_files;

struct curl_slist *prepare_headers(char *content_length)
{
	struct curl_slist *chunk = NULL;

	chunk = curl_slist_append(chunk, "Accept: application/json");
	chunk = curl_slist_append(chunk, "Content-Type: application/json");
	chunk = curl_slist_append(chunk, content_length);
	return chunk;
}

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
void list_dirs(const char *name, int level, int *num_per_level, void (*cb)(int, int, int), int num_at_1, int max_level)
{
    DIR *dir;
    struct dirent *entry;

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

            if (search_array(ignored, entry->d_name, 3)) {
                printf("Ignoring %s\n", path);
                continue;
            }
            num_dirs++;
            num_per_level[level]++;
            list_dirs(path, level + 1, num_per_level, cb, num_at_1, max_level);
        }
    } while ((entry = readdir(dir)));
    cb(level, num_at_1, num_per_level[1]);
    closedir(dir);
}

int get_file_size(char *path) {
    FILE *fd = fopen(path, "rb");
	int file_len;

	fseek(fd, 0, SEEK_END);
	file_len = ftell(fd);
    fclose(fd);
    return file_len;
}

// added count of dirs per level
// now handle callback
void list_dirs_files(const char *name, int level, int *num_per_level, void (*cb)(int, int, int), int num_at_2, char *parent_name, int max_level)
{
    DIR *dir;
    struct dirent *entry;
    char *ext;
    char path[1024];
    int len;
    json_t *json_str;
    // char *dir_name;

    // if (level < 3) dir_name = malloc(512);

    if ((max_level > -1) && (level == max_level)) return;

    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir))){
        closedir(dir);
        return;
    }

    do {
        len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
        if (len >= 1024) {
            printf("FATAL: %s %d\n", name, len);
        }
        path[len] = 0;
        json_str = json_string(path);
        json_array_append(all_files, json_str);

        if (entry->d_type == DT_DIR) {
            // todo group these
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            if (search_array(ignored, entry->d_name, 3)) {
                printf("Ignoring %s\n", path);
                continue;
            }
            num_dirs++;
            num_per_level[level]++;
            // if (level< 2) printf("nd: %d, nf: %d\n", num_dirs, num_files);
            // if (level < 4) printf("      %*s[%s] %s (%d)\n", level*2, "", entry->d_name, path, level);

            // if (level < 2) {
            //     c
            // }

            list_dirs_files(path, level + 1, num_per_level, cb, num_at_2, "", max_level);
        }
        else {
            ext = get_filename_ext( entry->d_name );
            // if( search_array( accepted_exts, ext, 3 ) ) {
            num_files++;
            // get_file_size(path);
            // if (!(num_files % 10000)) printf("nf: %d\n", num_files);
            // printf("%5d %*s- %s\n", num_files, level*2, "", entry->d_name);
            // }
        }
    } while ((entry = readdir(dir)));
    cb(level, num_at_2, num_per_level[2]);
    // if (level < 3) free(dir_name);
    closedir(dir);
}


void fun(int a, int b, int c) {
//    printf("Fun %d %d\n\n", a, b);
}

void fun2(int level, int b, int c) {
   printf("CB %d %d %d\n", level, b, c);
    // float pc = (float) ((c * 100.0f) / b);
    int pc = c * 1000 / b;

    if (level == 2) {
        printf("%.1f\n", pc / 10.0);
        send_percent_request(pc);
    }
}

void reset_npl(int *num_per_levels) {
    memset(num_per_levels, 0, 512 * sizeof(int));
}

void send_percent_request(int percent) {

    printf("send_percent\n");
    char *payload;
    payload = malloc(100);

    sprintf(payload, "{\"percent\": %.1f}\n\n\n", percent / 10.0);

    send_request("/scan-pc", payload);
    free(payload);
}

// https://stackoverflow.com/questions/22367580/libcurl-how-to-stop-output-to-command-line-in-c
void send_request(char * endpoint, char * payload) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
	char *content_length;
	char content_length_val[8];
    const char *base_url = "http://localhost:5000";
    char *url;
    int base_url_len = strlen(base_url);
    int url_len = base_url_len + strlen(endpoint);
    FILE *devnull = fopen("/dev/null", "w+");

    printf("send_request %s %d %d\n", base_url, base_url_len, url_len);
    url = malloc(url_len + 1);
    strcpy(url, base_url);
    strcat(url + base_url_len, endpoint);
    printf("URL        >>\t%s\n", url);

	curl = curl_easy_init();
	if (!curl)
	{
		printf("Could not init cURL, aborting!\n");
		return 1;
	}

	sprintf(content_length_val, "%ld", strlen(payload));
	content_length = malloc(17 + strlen(content_length_val));
	strcpy(content_length, "Content-Length: ");
	strcat(content_length, content_length_val);

	// printf("Content-Length >>\t%s\n", content_length);
	// printf("Payload        >>\t%s\n", payload);

	headers = prepare_headers(content_length);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, devnull);
    // curl_easy_setopt(curl, CURLOPT_MUTE, 1);
    // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    // curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
						curl_easy_strerror(res));

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	free(content_length);
}

int main(int argc, char **argv)
{
    time_t start;
    time_t end;
    double seconds;
    int MAX = 3;
    int num_per_level1[512];
    int num_per_level2[512];
    int num_at_1;
    int num_at_2;
    char *json_files;

    if(argc < 2) {
        printf("Not enough arguments:\n  ldir <dir>\n\n");
        exit(-1);
    }

    // start counting time
    start = time(NULL);

    // get numbers of dirs at level 1
    reset_npl(num_per_level1);
    list_dirs(argv[1], 0, num_per_level1, &fun, 0, 2);
    num_at_1 = num_per_level1[1];
    end = time(NULL);

    // recurse one level deeper
    reset_npl(num_per_level1);
    list_dirs(argv[1], 0, num_per_level1, &fun2, num_at_1, 3);
    num_at_2 = num_per_level1[2];

    // no limit, scan dirs & files
    num_files = 0;
    num_dirs = 0;
    all_files = json_array();

    reset_npl(num_per_level2);
    list_dirs_files(argv[1], 0, num_per_level2, &fun2, num_at_2, "", -1);
    end = time(NULL);

    seconds = difftime(end, start);

    printf("%d dirs scanned\n", num_dirs);
    for (int i = 0; i < MAX; i++) printf("\t%2d: %d\n", i, num_per_level1[i]);
    printf("%d files found\n", num_files);
    printf("%f seconds elapsed\n", seconds);

    printf("building and sending files\n");
    json_files = json_dumps(all_files, 0);
    printf("json payload size: %d\n", strlen(json_files));
    send_request("/files", json_files);
    free(json_files);

    return 0;
}

