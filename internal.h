/***
 * -*- linux-c  -*-
 * Copyright 2017 Nathaniel Clark <Nathaniel.Clark@misrule.us>
 * All Rights Reserved
 */
#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>

#include <purple.h>


#define PROG_NAME "irisd"

#define PRPL_ID "x-"PROG_NAME

#define DBUS_NAME "net.utopiabound.iris"

#define VERSION_STR "0.0.1"

enum IRISDataFlags {
	IRIS_FLAG_NONE=0,
	IRIS_FLAG_DEBUG=	0x01,	/**< Enable Debugging */
	IRIS_FLAG_SYSTEM=	0x10,	/**< GDBus connect to system D-Bus */
};

/**
 * Central info repository that is passed around.
 * All "R/O" data is set during init
 */
struct IRISData {
	PurpleAccount	*prplAccount;	/* Purple R/O */

	GDBusNodeInfo	*introspect;	/* GDBus R/O */
	guint		gdbusID;	/* GDBus R/O */

	GMainLoop	*loop;		/* Main R/O */
	guint		flags;		/* Main R/O */
};

/**** purple.c ****/
void init_purple(struct IRISData *, char *config_dir);
void close_purple(struct IRISData *);

void iris_send_message(PurpleBuddy *, const gchar *);

/*** gdbus.c ****/
void init_gdbus(struct IRISData *);
void close_gdbus(struct IRISData *);

#endif
