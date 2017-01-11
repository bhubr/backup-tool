#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "mp3_checksum.h"

void print_hex(char *s)
{
  while(*s) {
    printf("%02x", (unsigned char) (*s));
    s++;
  }
    
  printf("\n");
}

int main(int argc, char **argv) {
    unsigned char *mp3_hash_no_tags, *mp3_hash_with_tags;
    int cmp_result;

    mp3_hash_no_tags = mp3_checksum("sample/Cough.mp3");
    print_hex(mp3_hash_no_tags);

    mp3_hash_with_tags = mp3_checksum("sample/Cough_with_tags.mp3");
    print_hex(mp3_hash_with_tags);

    cmp_result = strcmp(mp3_hash_no_tags, mp3_hash_with_tags);

    free(mp3_hash_no_tags);
    free(mp3_hash_with_tags);

    assert(cmp_result == 0);

    return 0;
}