#include <jansson.h>

typedef struct response_wrapper {
  char **headers;
  int status_code;
  char *raw_body;
  json_t *json_body;  
} response;

typedef response *p_response;

response *send_request(char *host, int portno, char *method, char *path, char *data, char **headers);