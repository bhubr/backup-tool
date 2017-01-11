#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "post.h"
int main(int argc,char *argv[]) {
  char *result;
  char *headers[2];
  char *content_type = "Content-Type: application/x-www-form-urlencoded";
  headers[0] = malloc(strlen(content_type) + 1);
  strcpy(headers[0], content_type);
  headers[1] = 0;
  
  result = send_request("192.168.1.49", 8000, "POST", "/", "name=toto", headers);
  result = send_request("192.168.1.49", 8000, "GET", "/", "name=toto", headers);
}