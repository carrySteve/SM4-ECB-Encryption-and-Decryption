#ifdef LINKTIME

#include <stdio.h>
#include <stdlib.h>
#include "sm4.h"

size_t *__real_fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
size_t *__real_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

size_t *__wrap_fwrite(const void* buffer, size_t size, size_t count, FILE* stream)
{
	sm4_context ctx;

	// set the key
	unsigned char key[16] = "1111111111111111";

	// allocate memory
	unsigned char *input = alloca(100);
	unsigned char *output = alloca(100);


	long offset = ftell(stream);
	long left = offset;
	long leftBias = 0;
    int i;

    // offset the file header if it does not point to an address of 16-multiples bytes
	if(left % 16 != 0)
    {
        while(left % 16 != 0)
        {
            left--;
            leftBias++;
        }

        unsigned char *input_before = alloca(leftBias);


        fseek(stream, left, SEEK_SET);

        // read the previous encrypted text
        __real_fread(input_before, 1, leftBias, stream);

        unsigned char *result = alloca(leftBias);

        // decrypt the text
        sm4_setkey_dec(&ctx, key);
        sm4_crypt_ecb(&ctx, 0, leftBias, input_before, result);

        // take out the decrypted text
        for(i = 0; i < leftBias; i++)
        {
            input[i] = result[i];
        }
    }

    for(i = leftBias; i < count; i++)
    {
        input[i] = *(char*)buffer++;
    }

    count += leftBias;

    // pad zeros if the total text length is not of 16-multiples bytes
    i = 0;
    while(count % 16 != 0)
    {
        input[count + i] = '0';
        count++;
    }

    // encrypt the text
    sm4_setkey_enc(&ctx, key);
    sm4_crypt_ecb(&ctx, 0, count, input, output);

    // write the encrypted text to file
    fseek(stream, left, SEEK_SET);
    __real_fwrite(output, 1, count, stream);

    // reset the file pointer
    fseek(stream, left + count, SEEK_SET);

    return count;
}

size_t *__wrap_fread(const void* ptr, size_t size, size_t count, FILE* stream)
{
    sm4_context ctx;
	unsigned char key[16]= "1111111111111111";

	// allocate memory
	unsigned char *input = alloca(100);
	unsigned char *output = alloca(100);
	unsigned char *result = alloca(100);

	// total length including left and right offset
	int total_len;

	// input length
	int read_len;

	long offset = ftell(stream);
	long left = offset;

	long leftBias = 0;

	// offset the left pointer if it does not point to an address of 16-multiples bytes
	while(left % 16 != 0)
	{
		left--;
		leftBias++;
	}

	read_len = size * count;

	long right = offset + read_len;

	while(right % 16 != 0)
	{
		right++;
	}

	total_len = right - left;

	// read file
	fseek(stream, left, SEEK_SET);
	__real_fread(input, 1, total_len, stream);


	// decrypt the data
	sm4_setkey_dec(&ctx, key);
	sm4_crypt_ecb(&ctx, 0, total_len, input, result);

	int i;

	for(i = 0; i < read_len; i++)
	{
		output[i] = result[leftBias+i];
	}

	// copy to the input pointer
	memcpy(ptr, output, read_len);

	// reset the file pointer
	fseek(stream, offset + read_len, SEEK_SET);

	return read_len;
}


#endif