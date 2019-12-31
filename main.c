#include <stdio.h>
#include <stdlib.h>

int main()
{
	FILE* test = fopen("test.txt", "r+");

	// date to be written
	char* txt = "TJU SCS 3016218116 Huang Xingyu keep calm and carry on";

	if(test == NULL)
    {
        printf("No Such File");
        return 1;
    }

    // move file pointer to the end of the file
    fseek(test, 0L, SEEK_END);

    int read_len = strlen(txt);

    // write encrypted data
    fwrite(txt, sizeof(char), read_len, test);
    fclose(test);

    FILE* decry = fopen("test.txt", "r+");

    if(decry == NULL)
    {
        printf("No Such File");
        return 1;
    }

    char buffer[read_len];

    // read decrypted data
    fread(buffer, sizeof(char), read_len, decry);

    // print decrypted data
    printf("After decryption: %s\n", txt);
    fclose(decry);
    return 0;
}
