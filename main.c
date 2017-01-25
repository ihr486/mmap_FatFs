#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>
#include <stdbool.h>
#include <pthread.h>

#include "ff.h"

extern void disk_set_device_path(const char *devpath);
extern void disk_finalize(void);

void run_benchmark(int chunk_size)
{
  FATFS ff;
  FIL f;

  f_mount(&ff, "0:", 0);

  if (f_open(&f, "newfile.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK)
  {
    FILE *fp = fopen("big.txt", "rb");
    size_t read;
    UINT written;
    char *write_buf = (char *)malloc(chunk_size);
    while (1)
    {
      FRESULT res;
      read = fread(write_buf, 1, chunk_size, fp);
      if ((res = f_write(&f, write_buf, read, &written)) != FR_OK)
      {
	fprintf(stderr, "Failed to write to file: %d.\n", res);
	return;
      }
      if (read < chunk_size) break;
    }
    fclose(fp);

    f_close(&f);								/* Close the file */
  }
  else
  {
    fprintf(stderr, "Failed to open a new file.\n");
    return;
  }

  printf("%u ", chunk_size);
  disk_finalize();
}

static libusb_context *usb;
static libusb_device_handle *handle;
static pthread_t measurement_thread;
static volatile bool exit_flag = false;
static double total = 0;

static void *measurement_thread_func(void *arg)
{
  while (!exit_flag)
  {
    uint16_t buffer[32];
    int n;
    if (!libusb_interrupt_transfer(handle, 0x81, buffer, 64, &n, 0))
    {
      if (n >= 2)
      {
	for (int i = 0; i < (n - 2) / 2; i++)
	{
	  total += (int16_t)buffer[1 + i] * 0.625 * 1.1E-3;
	}
      }
    }
  }
}

int start_measurement(void)
{
    if (libusb_init(&usb))
    {
        fprintf(stderr, "Failed to initialize libusb.\n");
        return -1;
    }

    handle = libusb_open_device_with_vid_pid(usb, 0x0000, 0x0001);
    if (handle == NULL)
    {
        fprintf(stderr, "Failed to open the target.\n");
        return -1;
    }

    if (libusb_kernel_driver_active(handle, 0))
    {
        libusb_detach_kernel_driver(handle, 0);
    }

    if (libusb_claim_interface(handle, 0))
    {
        fprintf(stderr, "Failed to claim interface #0.\n");
        return -1;
    }
    
    pthread_create(&measurement_thread, NULL, measurement_thread_func, NULL);
    return 0;
}

void stop_measurement(void)
{
    exit_flag = true;
    pthread_join(measurement_thread, NULL);

    libusb_close(handle);
    libusb_exit(usb);
}

int main(int argc, const char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "Usage: fatfs [chunk size]\n");
    return EXIT_FAILURE;
  }

  int chunk_size = atoi(argv[1]);

  if (start_measurement())
    return EXIT_FAILURE;

  run_benchmark(chunk_size);

  stop_measurement();

  printf("Total power = %f[W]\n", total);

  return EXIT_SUCCESS;
}
