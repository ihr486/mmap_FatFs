/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "diskio.h"		/* FatFs lower layer API */

#define SECTOR_SIZE (512)

static struct
{
	DSTATUS status;
	int fd;
	const char *devpath;
	void *mapped_block;
	DWORD si_mapped, sc_mapped;
}
disk;

static DWORD sc_pagesize = 1;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	(void)pdrv;
	return disk.status;
}

void disk_set_device_path(const char *devpath)
{
	disk.devpath = devpath;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	(void)pdrv;
	disk.status = 0;
	disk.fd = open(disk.devpath, O_RDWR);
	disk.mapped_block = NULL;
	disk.si_mapped = 0;
	disk.sc_mapped = 0;
	sc_pagesize = sysconf(_SC_PAGESIZE) / SECTOR_SIZE;
	return disk.status;
}

static DRESULT diskio_remap(DWORD si_req, UINT sc_req)
{
  if (disk.mapped_block == NULL || si_req < disk.si_mapped || disk.si_mapped + disk.sc_mapped < si_req + sc_req)
  {
    if (disk.mapped_block != NULL)
      munmap(disk.mapped_block, SECTOR_SIZE * disk.sc_mapped);
    disk.si_mapped = si_req / sc_pagesize * sc_pagesize;
		disk.sc_mapped = ((si_req % sc_pagesize) + sc_req + sc_pagesize - 1) / sc_pagesize * sc_pagesize;
		disk.mapped_block = mmap(NULL, SECTOR_SIZE * disk.sc_mapped, PROT_READ | PROT_WRITE, MAP_SHARED, disk.fd, SECTOR_SIZE * disk.si_mapped);
    if (disk.mapped_block == (void *)-1)
      return RES_ERROR;
  }
  return RES_OK;
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
	DRESULT res;
	(void)pdrv;
	printf("READ request @ 0x%08X * %u\n", sector, count);
	if ((res = diskio_remap(sector, count)) == RES_OK)
  {
    void *src = (uint8_t *)disk.mapped_block + SECTOR_SIZE * (sector % sc_pagesize);
    memcpy(buff, src, SECTOR_SIZE * count);
    return RES_OK;
  }
  else
  {
    return res;
  }
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
	DRESULT res;
	(void)pdrv;
	printf("WRITE request @ 0x%08X * %u\n", sector, count);
	if ((res = diskio_remap(sector, count)) == RES_OK)
  {
    void *dest = (uint8_t *)disk.mapped_block + SECTOR_SIZE * (sector % sc_pagesize);
    memcpy(dest, buff, SECTOR_SIZE * count);
    return RES_OK;
  }
  else
  {
    return res;
  }
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
