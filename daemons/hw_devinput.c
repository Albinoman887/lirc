/****************************************************************************
 ** hw_devinput.c ***********************************************************
 ****************************************************************************
 *
 * receive keycodes input via /dev/input/...
 * 
 * Copyright (C) 2002 Oliver Endriss <o.endriss@gmx.de>
 *
 * Distribute under GPL version 2 or later.
 *
 */

/*
  TODO:

  - proper support for repeat
  - use more than 32 bits (?)
  
  CB
  
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <sys/fcntl.h>

#include <linux/input.h>

#include "hardware.h"
#include "ir_remote.h"
#include "lircd.h"
#include "receive.h"


static ir_code code;


int devinput_init()
{
	logprintf(LOG_INFO, "initializing '%s'", hw.device);
	
	if ((hw.fd = open(hw.device, O_RDONLY)) < 0) {
		logprintf(LOG_ERR, "unable to open '%s'", hw.device);
		return 0;
	}

	return 1;
}


int devinput_deinit(void)
{
	logprintf(LOG_INFO, "closing '%s'", hw.device);
	close(hw.fd);
	hw.fd=-1;
	return 1;
}


int devinput_decode(struct ir_remote *remote,
		    ir_code *prep, ir_code *codep, ir_code *postp,
		    int *repeat_flagp, lirc_t *remaining_gapp)
{
	logprintf(LOG_DEBUG, "devinput_decode");

	*prep = 0;
	*codep = code;
	*postp = 0;
	*repeat_flagp = 0;
	*remaining_gapp = 0;
	
	return 1;
}


char *devinput_rec(struct ir_remote *remotes)
{
	struct input_event event;
	int rd;


	logprintf(LOG_DEBUG, "devinput_rec");
	
	rd = read(hw.fd, &event, sizeof event);
	if (rd != sizeof event) {
		logprintf(LOG_ERR, "error reading '%s'", hw.device);
		return 0;
	}

	logprintf(LOG_DEBUG, "time %ld.%06ld  type %d  code %d  value %d",
		event.time.tv_sec, event.time.tv_usec,
		event.type, event.code, event.value);

	code = event.value ? 0x80000000 : 0;
	code |= ((event.type & 0x7fff) << 16);
	code |= event.code;

	logprintf(LOG_DEBUG, "code %.8llx", code);

	return decode_all(remotes);
}



struct hardware hw_devinput=
{
	"/dev/input/event0",	/* "device" */
	-1,			/* fd (device) */
	LIRC_CAN_REC_LIRCCODE,	/* features */
	0,			/* send_mode */
	LIRC_MODE_LIRCCODE,	/* rec_mode */
	32,			/* code_length */
	devinput_init,		/* init_func */
	NULL,			/* config_func */
	devinput_deinit,	/* deinit_func */
	NULL,			/* send_func */
	devinput_rec,		/* rec_func */
	devinput_decode,	/* decode_func */
	NULL,			/* readdata */
	"dev/input"
};
