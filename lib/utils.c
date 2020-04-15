/**
 *  Copyright (C) 2011-2012  Juho Vähä-Herttua
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

char *
utils_strsep(char **stringp, const char *delim)
{
    char *original;
    char *strptr;

    if (*stringp == NULL) {
        return NULL;
    }

    original = *stringp;
    strptr = strstr(*stringp, delim);
    if (strptr == NULL) {
        *stringp = NULL;
        return original;
    }
    *strptr = '\0';
    *stringp = strptr+strlen(delim);
    return original;
}

int
utils_read_file(char **dst, const char *filename)
{
    FILE *stream;
    int filesize;
    char *buffer;
    int read_bytes;

    /* Open stream for reading */
    stream = fopen(filename, "rb");
    if (!stream) {
        return -1;
    }

    /* Find out file size */
    fseek(stream, 0, SEEK_END);
    filesize = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    /* Allocate one extra byte for zero */
    buffer = malloc(filesize+1);
    if (!buffer) {
        fclose(stream);
        return -2;
    }

    /* Read data in a loop to buffer */
    read_bytes = 0;
    do {
        int ret = fread(buffer+read_bytes, 1,
                        filesize-read_bytes, stream);
        if (ret == 0) {
            break;
        }
        read_bytes += ret;
    } while (read_bytes < filesize);

    /* Add final null byte and close stream */
    buffer[read_bytes] = '\0';
    fclose(stream);

    /* If read didn't finish, return error */
    if (read_bytes != filesize) {
        free(buffer);
        return -3;
    }

    /* Return buffer */
    *dst = buffer;
    return filesize;
}

int
utils_hwaddr_raop(char *str, int strlen, const char *hwaddr, int hwaddrlen)
{
    int i,j;

    /* Check that our string is long enough */
    if (strlen == 0 || strlen < 2*hwaddrlen+1)
        return -1;

    /* Convert hardware address to hex string */
    for (i=0,j=0; i<hwaddrlen; i++) {
        int hi = (hwaddr[i]>>4) & 0x0f;
        int lo = hwaddr[i] & 0x0f;

        if (hi < 10) str[j++] = '0' + hi;
        else         str[j++] = 'A' + hi-10;
        if (lo < 10) str[j++] = '0' + lo;
        else         str[j++] = 'A' + lo-10;
    }

    /* Add string terminator */
    str[j++] = '\0';
    return j;
}

int
utils_hwaddr_airplay(char *str, int strlen, const char *hwaddr, int hwaddrlen)
{
    int i,j;

    /* Check that our string is long enough */
    if (strlen == 0 || strlen < 2*hwaddrlen+hwaddrlen)
        return -1;

    /* Convert hardware address to hex string */
    for (i=0,j=0; i<hwaddrlen; i++) {
        int hi = (hwaddr[i]>>4) & 0x0f;
        int lo = hwaddr[i] & 0x0f;

        if (hi < 10) str[j++] = '0' + hi;
        else         str[j++] = 'a' + hi-10;
        if (lo < 10) str[j++] = '0' + lo;
        else         str[j++] = 'a' + lo-10;

        str[j++] = ':';
    }

    /* Add string terminator */
    if (j != 0) j--;
    str[j++] = '\0';
    return j;
}

char *utils_parse_hex(const char *str, int str_len, int *data_len) {
    assert(str_len % 2 == 0);

    char *data = malloc(str_len / 2);

    for (int i = 0; i < (str_len / 2); i++) {
        char c_1 = str[i * 2];
        if (c_1 >= 97 && c_1 <= 102) {
            c_1 -= (97 - 10);
        } else if (c_1 >= 65 && c_1 <= 70) {
            c_1 -= (65 - 10);
        } else if (c_1 >= 48 && c_1 <= 57) {
            c_1 -= 48;
        } else {
            free(data);
            return NULL;
        }

        char c_2 = str[(i * 2) + 1];
        if (c_2 >= 97 && c_2 <= 102) {
            c_2 -= (97 - 10);
        } else if (c_2 >= 65 && c_2 <= 70) {
            c_2 -= (65 - 10);
        } else if (c_2 >= 48 && c_2 <= 57) {
            c_2 -= 48;
        } else {
            free(data);
            return NULL;
        }

        data[i] = (c_1 << 4) | c_2;
    }

    *data_len = (str_len / 2);
    return data;
}