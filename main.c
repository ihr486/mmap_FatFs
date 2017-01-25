#include <stdio.h>
#include <stdlib.h>

#include "ff.h"

extern void disk_set_device_path(const char *devpath);

int main(int argc, const char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "Please specify the path to block device.\n");
    return EXIT_FAILURE;
  }

  FATFS ff;
  FIL f;
  UINT bw;

  f_mount(&ff, "0:", 0);

  if (f_open(&f, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */

		f_write(&f, "It works!\r\n", 11, &bw);	/* Write data to the file */

		f_close(&f);								/* Close the file */
	}
  else
  {
    fprintf(stderr, "Failed to open a new file.\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
