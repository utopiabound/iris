#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>

#include <purple.h>


#define PROG_NAME "iris"

#define PRPL_ID "x-"PROG_NAME

#define DBUS_NAME "net.utopiabound."PROG_NAME

#define VERSION_STR "0.0.1"
#define DEFAULT_VERBOSITY 1

/**
 *
 * All "R/O" data is set during init
 */
struct IRISData {
    PurpleAccount	*prplAccount;	/* Purple R/O */
    GDBusNodeInfo	*introspect;	/* GDBus R/O */
    guint		gdbusID;	/* GDBus R/O */
    guint		verbosity;	/* Main R/O */
    
};

/**** purple.c ****/
void init_purple(struct IRISData *, char *config_dir);


/*** gdbus.c ****/
void init_gdbus(struct IRISData *);
void close_gdbus(struct IRISData *);

#endif
