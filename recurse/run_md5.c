#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 256

char * run_md5(char *file) {
    int len;
    char *cmd;
    int i = 0;
    int j = 5; // len of 'md5 "'
    len = strlen(file);
    cmd = malloc(27 + len);
    sprintf(cmd, "md5 \""); // %s'", file);
    for( ; i < len ; i++) {
        if(file[i] == '\"') {
            cmd[j] = '\\';
            j++;
            cmd[j] = '\"';
        }
        else {
            cmd[j] = file[i];
        }
        j++;
    }
    cmd[j++] = '\"';
    cmd[j] = 0;
    // strcpy(cmd + 6, file);
    // cmd[len + 4] = 0;
    // printf("cmd: %s\n", cmd);

    char tmp_buf[BUFSIZE];
    char buf[BUFSIZE];

    char *output = malloc(33);
    tmp_buf[0] = 0;

    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return NULL;
    }

    // sprintf(tmp_buf, "%s = ", cmd);
    while (fgets(buf, BUFSIZE, fp) != NULL) {

        // sprintf(output, "%s", buf + strlen(tmp_buf));
        // 
        strcat(tmp_buf, buf);
    }
    // printf("Output buf: %s\n", tmp_buf);
    // snprintf(output, 33, tmp_buf + 9 + len);
    memcpy(output, tmp_buf + 9 + len, 32);
    output[32] = 0;
    // printf("Output: %s\n", output);
    free(cmd);

    if(pclose(fp))  {
        printf("Command not found or exited with error status\n");
        return NULL;
    }

    return output;
}
