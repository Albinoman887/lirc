/*      $Id: mode2.c,v 5.8 2002/07/27 09:03:21 lirc Exp $      */

/****************************************************************************
 ** mode2.c *****************************************************************
 ****************************************************************************
 *
 * mode2 - shows the pulse/space length of a remote button
 *
 * Copyright (C) 1998 Trent Piepho <xyzzy@u.washington.edu>
 * Copyright (C) 1998 Christoph Bartelmus <lirc@bartelmus.de>
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "drivers/lirc.h"

int main(int argc,char **argv)
{
	int fd;
	lirc_t data;
	unsigned long mode;
	char *device=LIRC_DRIVER_DEVICE;
	char *progname;
	struct stat s;
	int dmode=0;

	progname="mode2";
	while(1)
	{
		int c;
		static struct option long_options[] =
		{
			{"help",no_argument,NULL,'h'},
			{"version",no_argument,NULL,'v'},
			{"device",required_argument,NULL,'d'},
			{"mode",no_argument,NULL,'m'},
			{0, 0, 0, 0}
		};
		c = getopt_long(argc,argv,"hvd:m",long_options,NULL);
		if(c==-1)
			break;
		switch (c)
		{
		case 'h':
			printf("Usage: %s [options]\n",progname);
			printf("\t -h --help\t\tdisplay this message\n");
			printf("\t -v --version\t\tdisplay version\n");
			printf("\t -d --device=device\tread from given device\n");
			printf("\t -m --mode\t\tenable alternative display mode\n");
			return(EXIT_SUCCESS);
		case 'v':
			printf("%s %s\n",progname, VERSION);
			return(EXIT_SUCCESS);
		case 'd':
			device=optarg;
			break;
		case 'm':
			dmode=1;
			break;
		default:
			printf("Usage: %s [options]\n",progname);
			return(EXIT_FAILURE);
		}
	}
	if (optind < argc-1)
	{
		fprintf(stderr,"%s: too many arguments\n",progname);
		return(EXIT_FAILURE);
	}
	
	fd=open(device,O_RDONLY);
	if(fd==-1)  {
		fprintf(stderr,"%s: error opening %s\n",progname,device);
		perror(progname);
		exit(EXIT_FAILURE);
	};

	if ( (fstat(fd,&s)!=-1) && (S_ISFIFO(s.st_mode)) )
	{
		/* can't do ioctls on a pipe */
	}
	else if(ioctl(fd,LIRC_GET_REC_MODE,&mode)==-1 || mode!=LIRC_MODE_MODE2)
	{
		printf("This program is only intended for receivers "
		       "supporting the pulse/space layer.\n");
		printf("Note that this is no error, but this program simply "
		       "makes no sense for your\n"
		       "receiver.\n");
		close(fd);
		exit(EXIT_FAILURE);
	}
	while(1)
	{
		int result;

		result=read(fd,&data,sizeof(data));
		if(result!=sizeof(data))
		{
			fprintf(stderr,"read() failed\n");
			break;
		}
		
		if (!dmode)
		{
			printf("%s %lu\n",(data&PULSE_BIT)?"pulse":"space",
			       (unsigned long) (data&PULSE_MASK));
		}
		else
		{
			static int bitno = 1;
			
			/* print output like irrecord raw config file data */
			printf(" %8lu" , data&PULSE_MASK);
			++bitno;
			if (data&PULSE_BIT)
			{
				if ((bitno & 1) == 0)
				{
					/* not in expected order */
					printf("-pulse");
				}
			}
			else
			{
				if (bitno & 1)
				{
					/* not in expected order */
					printf("-space");
				}
				if ( ((data&PULSE_MASK) > 50000) ||
				     (bitno >= 6) )
				{
					/* real long space or more
                                           than 6 codes, start new line */
					printf("\n");  
					if ((data&PULSE_MASK) > 50000)
						printf("\n");
					bitno = 0;
				}
			}
		}
		fflush(stdout);
	};
	return(EXIT_SUCCESS);
}
