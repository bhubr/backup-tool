#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 256

char * run_md5(char *file) {
    int len;
    char *cmd;
    len = strlen(file);
    cmd = malloc(7 + len);
    sprintf(cmd, "md5 \"%s\"", file);
    // strcpy(cmd + 6, file);
    // cmd[len + 4] = 0;
    // printf("cmd: %s", cmd);

    char tmp_buf[BUFSIZE];
    char buf[BUFSIZE];

    char *output = malloc(33);

    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return -1;
    }

    while (fgets(buf, BUFSIZE, fp) != NULL) {
        sprintf(tmp_buf, "MD5 (%s) = ", file);
        sprintf(output, "%s", buf + strlen(tmp_buf));
    }
    free(cmd);

    if(pclose(fp))  {
        printf("Command not found or exited with error status\n");
        return -1;
    }

    return output;
}
