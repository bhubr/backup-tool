#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "md5.h"

unsigned char *mp3_checksum(char *filepath) {
    MD5_CTX ctx;
    FILE *fp;
    char buffer[4];
    int start;
    int finish;
    int flags;
    int footer;
    int bs;
    int bs0, bs1, bs2, bs3;
    int datasize;
    unsigned char * databuf;
    unsigned char * result;

    // Open file
    fp = fopen(filepath, "rb");
    if(fp == NULL) {
        printf("ERR opening file\n");
        return 0;
    }

    // Search ID3v1 tag
    fseek(fp, -128, SEEK_END);
    finish = ftell(fp);

    fread(buffer, 3, 1, fp);
    buffer[3] = 0;

    if(strcmp((const char *)buffer, "TAG") == 0) {
        // printf("%s: found ID3v1 tag: %s\n", filepath, buffer);
        finish -= 128;
    }
    rewind(fp); // Done, rewind
    start = ftell(fp);

    // Search ID3v2 tag
    fread(buffer, 3, 1, fp);
    buffer[3] = 0;

    if(strcmp((const char *)buffer, "ID3") == 0) {

        // printf("%s: found ID3v2 tag: %sv2", filepath, buffer);

        // "major" version
        fread(buffer, 1, 1, fp);
        // printf(".%d", (int)buffer[0]);
        // "minor" version
        fread(buffer, 1, 1, fp);
        // printf(".%d\n", (int)buffer[0]);

        // flags
        fread(buffer, 1, 1, fp);
        flags = (int)buffer[0];
        footer = flags & (1 << 4);

        // body size
        fread(buffer, 4, 1, fp);
        bs0 = (int)buffer[0] & 127;
        bs1 = (int)buffer[1] & 127;
        bs2 = (int)buffer[2] & 127;
        bs3 = (int)buffer[3] & 127;
        bs = (bs0<<21) + (bs1<<14) + (bs2<<7) + bs3;

        fseek(fp, bs, SEEK_CUR);
        if(footer > 0) {
            fseek(fp, 10, SEEK_CUR);
        }
        start = ftell(fp);
    }

    fseek(fp, start, SEEK_SET);

    datasize = finish - start;
    databuf = malloc(datasize);
    fread(databuf, datasize, 1, fp);
    fclose(fp);

    result = malloc(17);
    result[16] = 0;
    MD5_Init(&ctx);
    MD5_Update(&ctx, databuf, datasize);
    MD5_Final(result, &ctx);

    free(databuf);
    return result;
}
