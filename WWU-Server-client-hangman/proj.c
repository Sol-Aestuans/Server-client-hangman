
#include <stdio.h>
#include <string.h>

#include "proj.h"

int read_stdin(char *buf, int buf_len, int *more) {
    // TODO: Copy your implementation from project 1.
    int count = 0;
    int keep_reading = 1;

    while (keep_reading) {

        int chr = fgetc(stdin);
        buf[count] = chr;
        count++;

        if ((count >= buf_len)) { // at most buf_len-1 characters
            keep_reading = 0;
            *more = 1;
        } else if (chr == '\n') { // until the newline character
            keep_reading = 0;
            *more = 0;
        }
    }
    // `\0` is appended to end the string.
    buf[count] = '\0';

    return count;
}
