#include <jansson.h>

typedef struct {
  char **headers;
  json_t *json_body;  
} response;

typedef response *p_response;

response *send_request(char *host, int portno, char *method, char *path, char *data, char **headers);