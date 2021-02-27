#include <curl/curl.h>
#include <dirent.h>
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
    // char *ext;

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
            // if (level == 1) printf("      %*s[%s]\n", level*2, "", entry->d_name);

            list_dirs(path, level + 1, num_per_level, cb, num_at_1, max_level);
        }
        // else {
        //     ext = get_filename_ext( entry->d_name );
        //     // if( search_array( accepted_exts, ext, 3 ) ) {
        //     num_files++;
        //     // printf("%5d %*s- %s\n", num_files, level*2, "", entry->d_name);
        //     // }
        // }
    } while ((entry = readdir(dir)));
    cb(level, num_at_1, num_per_level[1]);
    closedir(dir);
}

void fun(int a, int b, int c) {
//    printf("Fun %d %d\n\n", a, b);
}

void fun2(int a, int b, int c) {
//    printf("CB %d %d\n\n", a, b);
    // float pc = (float) ((c * 100.0f) / b);
    int pc = c * 1000 / b;

    if (a == 2) {
        printf("CB %d %d %d %d\n\n", a, b, c, pc);
        send_request(11, pc);
    }
}

void reset_npl(int *num_per_levels) {
    memset(num_per_levels, 0, 4 * sizeof(int));
}

void send_request(int a, int percent) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
	char *content_length;
    char *payload;
	char content_length_val[8];

	curl = curl_easy_init();
	if (!curl)
	{
		printf("Could not init cURL, aborting!\n");
		return 1;
	}

    payload = malloc(100);

    sprintf(payload, "{\"percent\": %.1f}\n\n\n", percent / 10.0);
	sprintf(content_length_val, "%ld", strlen(payload));
	content_length = malloc(17 + strlen(content_length_val));
	strcpy(content_length, "Content-Length: ");
	strcat(content_length, content_length_val);

	printf("Content-Length >>\t%s\n", content_length);
	printf("Payload        >>\t%s\n", payload);

	headers = prepare_headers(content_length);

	curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:5000/scan-pc");
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
						curl_easy_strerror(res));

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	free(content_length);
	free(payload);
}

int main(int argc, char **argv)
{
    time_t start;
    time_t end;
    double seconds;
    int MAX = 3;
    int num_per_level[4];
    int num_at_1;


    if(argc < 2) {
        printf("Not enough arguments:\n  ldir <dir>\n\n");
        exit(-1);
    }

    // start counting time
    start = time(NULL);

    // get numbers of dirs at level 1
    reset_npl(num_per_level);
    list_dirs(argv[1], 0, num_per_level, &fun, 0, 2);
    num_at_1 = num_per_level[1];
    end = time(NULL);

    // recurse one level deeper
    reset_npl(num_per_level);
    list_dirs(argv[1], 0, num_per_level, &fun2, num_at_1, 3);
    // num_at_1 = num_per_level[1];


    end = time(NULL);

    seconds = difftime(end, start);
    printf("%d dirs scanned\n", num_dirs);
    for (int i = 0; i < MAX; i++) printf("\t%2d: %d\n", i, num_per_level[i]);
    printf("%d files found\n", num_files);
    printf("%f seconds elapsed\n", seconds);


    return 0;
}

