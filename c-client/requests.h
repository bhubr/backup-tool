#include <time.h>

void send_request(char * endpoint, char * payload);
void send_percent_request(int percent, char *phase, char *drive_id);
void send_scan_start_request(char *id, char *label, time_t timestamp);
void send_files_stats_request(int dirs, int files, char *drive_id);