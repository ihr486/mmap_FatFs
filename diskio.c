/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <libusb.h>

#include "diskio.h"		/* FatFs lower layer API */
#include "usb_device.h"
#include "mass_storage.h"

#define SECTOR_SIZE (512)

DSTATUS status = STA_NOINIT;
unsigned long read_count = 0, read_sector_count = 0, write_count = 0, write_sector_count = 0;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	(void)pdrv;
	return status;
}

void disk_finalize(void)
{
  //printf("Read count = %lu, Read sector count = %lu\n", disk.read_count, disk.read_sector_count);
  //printf("Write count = %lu, Write sector count = %lu\n", disk.write_count, disk.write_sector_count);
  printf("%lu %lu %lu %lu\n", read_count, read_sector_count, write_count, write_sector_count);
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	(void)pdrv;

	if (usb_device_open())
	{
	  return status;
	}
	if (mass_storage_inquiry())
	{
	  return status;
	}
	while (1)
	{
	  int response = mass_storage_test_unit_ready();
	  if (response == 0) break;
	}
	status &= ~STA_NOINIT;
	return status;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	(void)pdrv;
	//printf("READ request @ 0x%08lX * %u\n", sector, count);

	read_count++;
	read_sector_count += count;

	if (mass_storage_read(buff, sector, count))
	{
	  return RES_ERROR;
	}
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	(void)pdrv;
	//printf("WRITE request @ 0x%08lX * %u\n", sector, count);

	write_count++;
	write_sector_count += count;

	if (mass_storage_write(buff, sector, count))
	{
	  return RES_ERROR;
	}
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	(void)pdrv;
	switch(cmd)
	{
	case CTRL_SYNC:
		return RES_OK;
	case GET_SECTOR_SIZE:
		*(WORD *)buff = SECTOR_SIZE;
		return RES_OK;
	case CTRL_TRIM:
		return RES_OK;
	}
	return RES_PARERR;
}

DWORD get_fattime(void)
{
	union
	{
		struct {
			DWORD second : 5;
			DWORD minute : 6;
			DWORD hour : 5;
			DWORD day : 5;
			DWORD month : 4;
			DWORD year : 7;
		};
		DWORD value;
	} fattime;
	time_t t = time(NULL);
	struct tm *lt;
	lt = localtime(&t);
	fattime.year = lt->tm_year - 80;
	fattime.month = lt->tm_mon + 1;
	fattime.day = lt->tm_mday;
	fattime.hour = lt->tm_hour;
	fattime.minute = lt->tm_min;
	fattime.second = lt->tm_sec / 2;
	return fattime.value;
}
