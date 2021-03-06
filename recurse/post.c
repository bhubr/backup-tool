#include <stdbool.h> /* printf, sprintf */
#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit, atoi, malloc, free */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
// #include <jansson.h>
#include "utils.h"
#include "post.h"

#define BUF_SIZE 8192
void error(const char *msg) { perror(msg); exit(0); }

struct json_t* parse_body_json(char *body) {
    json_error_t err;
    json_t *obj;
    const char *key;
    json_t *value;

    obj = json_loads(body, JSON_DECODE_ANY, &err);
    if(!obj) {
        fprintf(stderr, "error: on line %d: %s\n", err.line, err.text);
        return NULL;
    }

    // json_object_foreach(obj, key, value) {
    //     switch( json_typeof(value) ) {
    //         case JSON_INTEGER:
    //             printf("%s => %d\n", key, json_integer_value(value));
    //             break;
    //         case JSON_STRING:
    //         default:
    //             printf("%s => %s\n", key, json_string_value(value));
    //     }
    // }
    return obj;
}

p_response send_request(char *host, int portno, char *method, char *path, char *data, char **headers)
{
    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total, message_size;
    char *header;
    char *message, *response = NULL, *response_ptr;
    char *eol;
    int i = 0, j;
    int header_len;
    char **response_headers;
    char status_code[5];
    p_response response_obj = NULL;
    struct response_wrapper re;
    // for(j = 0; j < 2 ; j++) {
    //     if(headers[j] == 0) continue;
    //     printf("header %d  => %s (ptr: %p)\n", j, headers[j], headers[j]);
    // }
    // if(headers[1] != 0) print_hex((unsigned char *)headers[1]);

    response = malloc(BUF_SIZE);
    response_obj = malloc(sizeof(re));

    // printf("post data: %s\n", data);
    // if(headers[1] != 0) {
    //     printf("#1 "); print_hex(headers[1]);
    // }

    /* How big is the message? */
    message_size=0;
    if(!strcmp(method,"GET"))
    {
        message_size+=strlen("%s %s%s%s HTTP/1.0\r\n");        /* method         */
        message_size+=strlen(method);                          /* path           */
        message_size+=strlen(path);                            /* headers        */
        if( data != NULL)
            message_size+=strlen(data);                        /* query string   */
        if (headers != NULL) {
            while((header = headers[i])) {
                message_size+=strlen(header)+strlen("\r\n");
                i++;
            }
            i = 0;
        }
        message_size+=strlen("\r\n");                           /* blank line     */
    }
    else
    {
        message_size+=strlen("%s %s HTTP/1.0\r\n");
        message_size+=strlen(method);                            /* method         */
        message_size+=strlen(path);                              /* path           */
        if (headers != NULL) {                                   /* headers        */
            while((header = headers[i])) {
                message_size+=strlen(header)+strlen("\r\n");
                i++;
            }
            i = 0;
            message_size+=strlen("Content-Length: %d\r\n")+10;   /* content length */
        }
            
        if( data != NULL)
            message_size+=strlen(data);                          /* body           */
    }
    // printf("message size: %d\n", message_size);

    /* allocate space for the message */
    message=malloc(message_size);
    // if(headers[1] != 0) {
    //     printf("#2 "); print_hex(headers[1]);
    // }

    /* fill in the parameters */
    if(!strcmp(method,"GET"))
    {
        if( data != NULL)
            sprintf(message,"%s %s%s%s HTTP/1.0\r\n",
                strlen(method)>0?method:"GET",               /* method         */
                strlen(path)>0?path:"/",                     /* path           */
                strlen(data)>0?"?":"",                       /* ?              */
                strlen(data)>0?data:"");                     /* query string   */
        else
            sprintf(message,"%s %s HTTP/1.0\r\n",
                strlen(method)>0?method:"GET",               /* method         */
                strlen(path)>0?path:"/");                    /* path           */
        if (headers != NULL) {                               /* headers        */
            while((header = headers[i])) {
                strcat(message,header);strcat(message,"\r\n");
                i++;
            }
            i = 0;
        }

        strcat(message,"\r\n");                                /* blank line     */
    }
    else
    {
        sprintf(message,"%s %s HTTP/1.0\r\n",
            strlen(method)>0?method:"POST",                    /* method         */
            strlen(path)>0?path:"/");                          /* path           */
        if (headers != NULL) {
            while((header = headers[i])) {
                strcat(message,header);strcat(message,"\r\n");
                i++;
            }
            i = 0;
        }
        if( data != NULL)
            sprintf(message+strlen(message),"Content-Length: %lu\r\n",strlen(data));
        strcat(message,"\r\n");                                /* blank line     */
        if( data != NULL)
            strcat(message,data);                           /* body           */
    }

    /* What are we going to send? */
    // printf("Request: %s\n", message);

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) error("ERROR, no such host");

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd,message+sent,total-sent);
        if (bytes < 0)
            error("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent+=bytes;
    } while (sent < total);

    /* receive the response */
    memset(response,0,BUF_SIZE);
    total = BUF_SIZE-1;
    received = 0;
    do {
        bytes = read(sockfd,response+received,total-received);
        if (bytes < 0)
            error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received+=bytes;
    } while (received < total);

    if (received == total) {
        error("ERROR storing complete response from socket");
        printf("%s\n",response);
    }

    /* close the socket */
    close(sockfd);
    free(message);

    // if(headers[1] != 0) {
    //     printf("#3a "); print_hex(headers[1]);
    // }


    response_headers = malloc(50 * sizeof(char *));
    response_ptr = response;
    // if(headers[1] != 0) {
    //     printf("#3b %p => ", headers[1]); print_hex(headers[1]);
    // }

    i = 0;
    // print_hex(response_ptr);
    do {
        eol = strchr(response_ptr, '\r');
        header_len = eol - response_ptr;
        if(header_len <= 1) break;
        response_headers[i] = malloc(header_len + 1);
        memcpy(response_headers[i], response_ptr, header_len );
        response_headers[i][header_len] = 0;
        // printf("#%d len: %d %s %p %s\n", i, header_len, response_headers[i], response_headers[i], headers[1]);
        i++;
        response_ptr = eol + 2;
    } while(true);
    // if(headers[1] != 0) {
    //     printf("#4a %p => ", headers[1]); print_hex(headers[1]);
    // }
    response_headers[i] = 0;

    response_ptr = response;
    while(*response_ptr != '{' && *response_ptr != 0) response_ptr++;

    snprintf(status_code, 4, "%s", response_headers[0] + 9);
    response_obj->status_code = atoi(status_code);
    // printf("status txt: %s, int: %d, hex: ", status_code, response_obj->status_code);
    // print_hex(status_code);
    response_obj->raw_body = response;
    response_obj->headers = response_headers;
    response_obj->json_body = parse_body_json(response_ptr);
    // if(headers[1] != 0) {
    //     printf("#4b "); print_hex(headers[1]);
    // }

    if(response_obj->status_code != 200) {
        json_t *error_j = json_object_get(response_obj->json_body, "error");
        printf("\n#######\n# Encountered error (status %d): %s\n#######\n", response_obj->status_code, json_string_value(error_j));
        json_decref(error_j);
        return NULL;
    }
    else {
        json_t *id_j = json_object_get(response_obj->json_body, "id");
    }
    // if(headers[1] != 0) {
    //     printf("#5 "); print_hex(headers[1]);
    // }
    return response_obj;
}
