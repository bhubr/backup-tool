#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 128

int run_md5(char *file) {
    int len;
    char *cmd;
    len = strlen(file);
    cmd = malloc(5 + len);
    strcpy(cmd, "md5 ");
    strcpy(cmd + 4, file);
    cmd[len + 4] = 0;
     // = "md5 client";

    char buf[BUFSIZE];
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return -1;
    }

    while (fgets(buf, BUFSIZE, fp) != NULL) {
        // Do whatever you want here...
        // printf("OUTPUT: %s", buf);
    }
    free(cmd);

    if(pclose(fp))  {
        printf("Command not found or exited with error status\n");
        return -1;
    }

    return 0;
}

// int main(int argc, char **argv) {
//     parse_output();
// }
