#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{

    FILE  *fp[2]; /* 0-input, 1-output */

    char  c;

    /* Check for command line input */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source-file> <destination-file>\n", argv[0]);
        return -1;
    }

    /* Open the source file for reading */
    fp[0] = fopen(argv[1], "r");
    if (fp[0] == NULL) {
        fprintf(stderr, "Error unable to open file '%s', errno = %d (%s) \n",
                argv[1], errno, strerror(errno));
        return -1;
    }

    /* Open the destination for writing */
    fp[1] = fopen(argv[2], "w");
    if (fp[1] == NULL) {
        fprintf(stderr, "Error unable to open file '%s', errno = %d (%s) \n",
                argv[2], errno, strerror(errno));
        return -1;
    }

    /* read the contents of the source file and write to the
     * destination file one byte at a time */
    while ((c = fgetc(fp[0])) != EOF) {
        fputc(c, fp[1]);
    }

    printf("\nCopied file contents to %s \n", argv[2]);

   fclose(fp[0]);
   fclose(fp[1]);

   return 0;

}
