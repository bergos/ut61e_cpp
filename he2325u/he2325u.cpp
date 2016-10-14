/*******************************************************
 UT61E / HOITEK HE2325U USB interface SW
 by Rainer Wetzel (c) 2011 (diyftw.de)

 Based on:
     hidtest.cpp - Windows HID simplification

     Alan Ott
     Signal 11 Software

     8/22/2009

     Copyright 2009, All Rights Reserved.
 
     This contents of this file may be used by anyone
     for any reason without any conditions and may be
     used as a starting point for your own applications
     which use HIDAPI.

 As well as on this for the decode of the raw data:
     hoitek.cpp
     by Bernd Herd (C) 2009 (herdsoft.com)

     Based on:
     tenma.c
     by Robert Kavaler (c) 2009 (relavak.com)

     Utility program to read daata from a Houtek HID UART device.
     This device is used in the USB cable for PeakTech 3315 DMM devices
     and Likely Tinma 72-7730 DMM.

     Based on linux /dev/hidraw interface.

********************************************************/

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <hidapi/hidapi.h>

// Headers needed for sleeping.
#ifdef _WIN32
	#include <windows.h>
#else
	#include <fcntl.h>
	#include <unistd.h>
	#include <sys/ioctl.h>
	#include <linux/usbdevice_fs.h>
#endif

#define MAX_STR 255

void send_usb_reset(char* filename) {
	int fd = open(filename, O_WRONLY);

	if (fd < 0)
		return;

	int rc = ioctl(fd, USBDEVFS_RESET, 0);

	if (rc < 0)
		return;

	close(fd);
}

void hid_info_to_path(hid_device_info* dev, char* buf, int size) {
	int bus;
	int device;

	sscanf(dev->path, "%x:%x", &bus, &device);

	snprintf(buf, size, "/dev/bus/usb/%03d/%03d", bus, device);
}

int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[256];
	hid_device *handle=0;
	int i;
	int dev_cnt = 0;

#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#endif

	struct hid_device_info *devs, *cur_dev;

	if(argc==1) // if no params are supplied list available devs
	{
		devs = hid_enumerate(0x1a86, 0xe008); // all chips this SW belongs to...
		cur_dev = devs;
		while(cur_dev){cur_dev = cur_dev->next; dev_cnt++; }
		printf("[!] found %i devices:\n", dev_cnt);

		cur_dev = devs;
		while (cur_dev) {
			hid_info_to_path(cur_dev, (char*)buf, MAX_STR);
			printf("\t%s\t%s\n", cur_dev->path, buf);
			cur_dev = cur_dev->next;
		}

		hid_free_enumeration(devs);
	}


	if(argc>1) // use the supplied device (if any)
	{
		send_usb_reset(argv[1]);
		handle = hid_open_path(argv[1]);
	}

	// Open the device using the VID, PID,
	// and optionally the Serial number (NULL for the hoitek chip).
	if(handle==0) {
		// buf contains the last device path
		send_usb_reset((char*)buf);

		handle = hid_open(0x1a86, 0xe008, NULL); // 1a86 e008
	}

	if (!handle) {
		printf("unable to open device\n");
		return 1;
	}

	memset(buf,0x00,sizeof(buf));

	unsigned int bps = 19200;
	// Send a Feature Report to the device
	buf[0] = 0x0; // report ID
	buf[1] = bps;
	buf[2] = bps>>8;
	buf[3] = bps>>16;
	buf[4] = bps>>24;
	buf[5] = 0x03; // 3 = enable?
	res = hid_send_feature_report(handle, buf, 6); // 6 bytes
	if (res < 0) {
		printf("Unable to send a feature report.\n");
	}

	memset(buf,0,sizeof(buf));

	printf("-data start-\n");

	usleep(1000);

	do
	{
		res = 0;
		while (res == 0)
		{
			res = hid_read(handle, buf, sizeof(buf));
			if (res == 0)
				printf("waiting...\n");
			if (res < 0)
				printf("Unable to read()\n");
		}

		// format data 
		int len=buf[0] & 7; // the first byte contains the length in the lower 3 bits ( 111 = 7 )
		for (i=0; i<len; i++)
			buf[i+1] &= 0x7f; // bitwise and with 0111 1111, mask the upper bit which is always 1

		if(len>0)
		{
			fwrite(buf+1, 1, len, stdout); // write data directly to stdout to enable pipeing to interpreter app
			fflush(stdout);
		}
	}while(res>=0);
	//  printf("%s",hid_error(handle));
	hid_close(handle);

#ifdef WIN32
	system("pause");
#endif

	return 0;
}
