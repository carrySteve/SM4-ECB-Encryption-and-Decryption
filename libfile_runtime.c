
#ifdef RUNTIME
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "sm4.h"

size_t fwrite(const void *buffer, size_t size, size_t count, FILE *stream)
{
    size_t (*fwritep)(const void *buffer, size_t size, size_t count, FILE *stream);
    char *error;

    fwritep = dlsym(RTLD_NEXT, "fwrite");
    if ((error = dlerror()) != NULL)
    {
        fputs(error, stderr);
        exit(1);
    }

    sm4_context ctx;

    unsigned char key[16] = "1111111111111111";

    unsigned char *input = alloca(100);
    unsigned char *output = alloca(100);

    long offset = ftell(stream);
    long left = offset;
    long leftBias = 0;
    int i;

    if (left % 16 != 0)
    {
        while (left % 16 != 0)
        {
            left--;
            leftBias++;
        }

        unsigned char *input_before = alloca(leftBias);

        fseek(stream, left, SEEK_SET);

        fread(input_before, 1, leftBias, stream);

        unsigned char *result = alloca(leftBias);

        sm4_setkey_dec(&ctx, key);
        sm4_crypt_ecb(&ctx, 0, leftBias, input_before, result);

        for (i = 0; i < leftBias; i++)
        {
            input[i] = result[i];
        }
    }

    for (i = leftBias; i < count; i++)
    {
        input[i] = *(char *)buffer++;
    }

    i = 0;
    while (count % 16 != 0)
    {
        input[count + i] = '0';
        count++;
    }

    sm4_setkey_enc(&ctx, key);
    sm4_crypt_ecb(&ctx, 0, count, input, output);

    fseek(stream, left, SEEK_SET);
    fwritep(output, 1, count, stream);

    fseek(stream, left + count, SEEK_SET);

    return count;
}

size_t fread(void *ptr, size_t size, size_t count, FILE *stream)

{
    size_t (*freadp)(void *ptr, size_t size, size_t nmemb, FILE *stream);
    char *error;

    freadp = dlsym(RTLD_NEXT, "fread");
    if ((error = dlerror()) != NULL)
    {
        fputs(error, stderr);
        exit(1);
    }

    sm4_context ctx;
    unsigned char key[16] = "1111111111111111";

    unsigned char *input = alloca(100);
    unsigned char *output = alloca(100);
    unsigned char *result = alloca(100);

    int total_len;
    int read_len;

    long offset = ftell(stream);
    long left = offset;

    long leftBias = 0;

    while (left % 16 != 0)
    {
        left--;
        leftBias++;
    }

    read_len = size * count;

    long right = offset + read_len;

    while (right % 16 != 0)
    {
        right++;
    }

    total_len = right - left;

    fseek(stream, left, SEEK_SET);
    freadp(input, 1, total_len, stream);

    sm4_setkey_dec(&ctx, key);
    sm4_crypt_ecb(&ctx, 0, total_len, input, result);

    int i;

    for (i = 0; i < read_len; i++)
    {
        output[i] = result[leftBias + i];
    }

    memcpy(ptr, output, read_len);

    fseek(stream, offset + read_len, SEEK_SET);

    return read_len;
}

#endif