#include <stdio.h>
#include <stdlib.h>

#include "ff.h"

extern void disk_set_device_path(const char *devpath);
extern void disk_finalize(void);

int main(int argc, const char *argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: fatfs [block device] [chunk size]\n");
    return EXIT_FAILURE;
  }

  FATFS ff;
  FIL f;
  UINT bw;

  disk_set_device_path(argv[1]);

  int chunk_size = atoi(argv[2]);

  f_mount(&ff, "0:", 0);

  if (f_open(&f, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
  {
    FILE *fp = fopen("big.txt", "rb");
    size_t read, written;
    char *write_buf = (char *)malloc(chunk_size);
    while (1)
    {
      FRESULT res;
      read = fread(write_buf, 1, chunk_size, fp);
      if ((res = f_write(&f, write_buf, read, &written)) != FR_OK)
      {
	fprintf(stderr, "Failed to write to file: %d.\n", res);
	return -1;
      }
      if (read < chunk_size) break;
    }
    fclose(fp);

    f_close(&f);								/* Close the file */
  }
  else
  {
    fprintf(stderr, "Failed to open a new file.\n");
    return EXIT_FAILURE;
  }

  printf("%u ", chunk_size);
  disk_finalize();

  return EXIT_SUCCESS;
}
