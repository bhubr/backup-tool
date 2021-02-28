#include <curl/curl.h>
#include <jansson.h>
#include <string.h>

struct curl_slist *prepare_headers(char *content_length)
{
	struct curl_slist *chunk = NULL;

	chunk = curl_slist_append(chunk, "Accept: application/json");
	chunk = curl_slist_append(chunk, "Content-Type: application/json");
	chunk = curl_slist_append(chunk, content_length);
	return chunk;
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
		return;
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
	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
						curl_easy_strerror(res));

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	free(content_length);
}

void send_percent_request(int percent, char *phase, char *drive_id) {
    char *payload;
    payload = malloc(100);

    sprintf(payload, "{\"id\": \"%s\", \"phase\": \"%s\", \"percent\": %.1f}\n\n\n", drive_id, phase, percent / 10.0);

    send_request("/scan-pc", payload);
    free(payload);
}

void send_scan_start_request(char *id, char *label, time_t timestamp) {
    json_t *data = json_object();
    json_t *val;
    char *payload;

    val = json_string(id);
    json_object_set(data, "id", val);
    json_decref(val);

    val = json_string(label);
    json_object_set(data, "label", val);
    json_decref(val);

    val = json_integer(timestamp);
    json_object_set(data, "timestamp", val);
    json_decref(val);

    payload = json_dumps(data, 0);
    send_request("/scan-start", payload);
    free(payload);
    json_decref(data);
}

void send_files_stats_request(int dirs, int files, char *drive_id) {
    char *payload;
    payload = malloc(100);

    sprintf(payload, "{\"id\": \"%s\", \"dirs\": %d, \"files\": %d}\n\n\n", drive_id, dirs, files);

    send_request("/scan-files-stats", payload);
    free(payload);
}