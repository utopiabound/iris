/***
 * -*- linux-c  -*-
 * Copyright 2017 Nathaniel Clark <Nathaniel.Clark@misrule.us>
 * All Rights Reserved
 */

#include "internal.h"

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string.h>
#ifdef _WIN32
#  include <conio.h>
#  include "win32/win32dep.h"
#else
#  include <unistd.h>
#endif


#define PLUGIN_SAVE_PREF "/"PROG_NAME"/plugins/loaded"

/**
 * The following eventloop functions are used in both pidgin and purple-text. If your
 * application uses glib mainloop, you can safely use this verbatim.
 */
#define PURPLE_GLIB_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define PURPLE_GLIB_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

typedef struct _PurpleGLibIOClosure {
	PurpleInputFunction function;
	guint result;
	gpointer data;
} PurpleGLibIOClosure;

static void purple_glib_io_destroy(gpointer data)
{
	g_free(data);
}

static gboolean purple_glib_io_invoke(GIOChannel *source, GIOCondition condition, gpointer data)
{
	PurpleGLibIOClosure *closure = data;
	PurpleInputCondition purple_cond = 0;

	if (condition & PURPLE_GLIB_READ_COND)
		purple_cond |= PURPLE_INPUT_READ;
	if (condition & PURPLE_GLIB_WRITE_COND)
		purple_cond |= PURPLE_INPUT_WRITE;

	closure->function(closure->data, g_io_channel_unix_get_fd(source),
			  purple_cond);

	return TRUE;
}

static guint glib_input_add(gint fd, PurpleInputCondition condition,
			    PurpleInputFunction function, gpointer data)
{
	PurpleGLibIOClosure *closure = g_new0(PurpleGLibIOClosure, 1);
	GIOChannel *channel;
	GIOCondition cond = 0;

	closure->function = function;
	closure->data = data;

	if (condition & PURPLE_INPUT_READ)
		cond |= PURPLE_GLIB_READ_COND;
	if (condition & PURPLE_INPUT_WRITE)
		cond |= PURPLE_GLIB_WRITE_COND;

#if defined _WIN32 && !defined WINPIDGIN_USE_GLIB_IO_CHANNEL
	channel = wpurple_g_io_channel_win32_new_socket(fd);
#else
	channel = g_io_channel_unix_new(fd);
#endif
	closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT,
		cond, purple_glib_io_invoke, closure, purple_glib_io_destroy);

	g_io_channel_unref(channel);
	return closure->result;
}

static PurpleEventLoopUiOps glib_eventloops =
{
	.timeout_add=		g_timeout_add,
	.timeout_remove=	g_source_remove,
	.input_add=		glib_input_add,
#if GLIB_CHECK_VERSION(2,14,0)
	.timeout_add_seconds=	g_timeout_add_seconds,
#endif
	.input_remove=		g_source_remove
};
/*** End of the eventloop functions. ***/

/*** Conversation uiops ***/
static void create_conv_cb(PurpleConversation *conv)
{
	g_debug("%s: (%s) Disable Logging\n", __func__,
		purple_conversation_get_name(conv));
	purple_conversation_set_logging(conv, FALSE);
}

static void write_conv_cb(PurpleConversation *conv,
			  const char *who,
			  const char *alias,
			  const char *message,
			  PurpleMessageFlags flags,
			  time_t mtime)
{
	const char *name;
	if (alias && *alias)
		name = alias;
	else if (who && *who)
		name = who;
	else
		name = NULL;

	g_debug("(%s) %s %s: %s\n", purple_conversation_get_name(conv),
		  purple_utf8_strftime("(%H:%M:%S)", localtime(&mtime)),
		  name, message);
}

static PurpleConversationUiOps null_conv_uiops =
{
	.create_conversation=	create_conv_cb,
	.write_conv=		write_conv_cb
};

static void
null_ui_init(void)
{
	/**
	 * This should initialize the UI components for all the modules. Here we
	 * just initialize the UI for conversations.
	 */
	purple_conversations_set_ui_ops(&null_conv_uiops);
}

/** Called by purple_core_get_ui_info(); should return the information
 *  documented there.
 */
static GHashTable *ui_info = NULL;
static GHashTable *ui_get_info_cb(void)
{
	g_debug("%s CALLED",__func__);
	if (ui_info == NULL) {
		ui_info = g_hash_table_new(g_str_hash, g_str_equal);

		g_hash_table_insert(ui_info, "name", PROG_NAME);
		g_hash_table_insert(ui_info, "version", VERSION_STR);
		//g_hash_table_insert(ui_info, "website", "https://pidgin.im");
		//g_hash_table_insert(ui_info, "dev_website", "https://developer.pidgin.im");
		g_hash_table_insert(ui_info, "client_type", "daemon");
	}

	return ui_info;
}

static PurpleCoreUiOps null_core_uiops =
{
	.ui_init= null_ui_init,
	.get_ui_info= ui_get_info_cb
};

#if 0
static void
signed_on(PurpleConnection *gc, gpointer null)
{
	PurpleAccount *account = purple_connection_get_account(gc);
	printf("Account connected: %s %s\n", purple_account_get_username(account), purple_account_get_protocol_id(account));
}
static void
connect_to_signals_for_demonstration_purposes_only(void)
{
	static int handle;
	purple_signal_connect(purple_connections_get_handle(), "signed-on", &handle,
				PURPLE_CALLBACK(signed_on), NULL);
}
#endif

void iris_send_message(PurpleBuddy *buddy, const gchar *msg)
{
	PurpleConversation *conv;
	PurpleConvIm *im;
	PurpleAccount *account = purple_buddy_get_account(buddy);

	g_debug("%s: Telling %s: %s\n",__func__,
		  purple_buddy_get_name(buddy), msg);

	conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM,
						     purple_buddy_get_name(buddy),
						     account);
	if (conv == NULL)
		conv = purple_conversation_new(PURPLE_CONV_TYPE_IM,
					       account,
					       purple_buddy_get_name(buddy));
	if (conv == NULL) {
		g_error("%s: Failed to find converstation for %s on account %s\n",
			__func__, purple_buddy_get_name(buddy),
			purple_account_get_name_for_display(account));
		return;
	}
	im = purple_conversation_get_im_data(conv);
	purple_conv_im_send(im, msg);
}

void init_purple(struct IRISData *info, char *config_dir) {
	char *path;
	PurpleSavedStatus *status;

	/* set a user-specified config directory */
	if (config_dir != NULL) {
		if (g_path_is_absolute(config_dir)) {
			purple_util_set_user_dir(config_dir);
		} else {
			/* Make an absolute (if not canonical) path */
			char *cwd = g_get_current_dir();
			path = g_build_path(G_DIR_SEPARATOR_S, cwd,
					    config_dir, NULL);
			purple_util_set_user_dir(path);
			g_free(path);
			g_free(cwd);
		}

		g_free(config_dir);
	}

	/* We don't want debug-messages to show up and corrupt the display */
	purple_debug_set_enabled((info->verbosity > DEFAULT_VERBOSITY));

	purple_core_set_ui_ops(&null_core_uiops);

	/* Set the uiops for the eventloop. If your client is glib-based, you can safely
	 * copy this verbatim. */
	purple_eventloop_set_ui_ops(&glib_eventloops);

	/* Now that all the essential stuff has been set, let's try to init the core. It's
	 * necessary to provide a non-NULL name for the current ui to the core. This name
	 * is used by stuff that depends on this ui, for example the ui-specific plugins. */
	if (!purple_core_init(PRPL_ID)) {
		/* Initializing the core failed. Terminate. */
		fprintf(stderr,
			"libpurple initialization failed. Dumping core.\n"
			"Please report this!\n");
		abort();
	}

	/* Create and load the buddylist. */
	purple_set_blist(purple_blist_new());
	//purple_blist_load();

	/* Load the preferences. */
	purple_prefs_load();

	//purple_plugins_load_saved(PLUGIN_SAVE_PREF);

	/* Load the pounces. */
	//purple_pounces_load();

	/** Account Login **/
	/* XYZZY - Load from config */
	info->prplAccount = purple_account_new("utopianet", "prpl-bonjour");
	purple_account_set_user_info(info->prplAccount, "Local Network Status Daemon");

	/* It's necessary to enable the account first. */
	purple_account_set_enabled(info->prplAccount, PRPL_ID, TRUE);

	/* Things to be set after enabled */
	purple_account_set_public_alias(info->prplAccount, "UtopiaNet", NULL, NULL);

	/* Now, to connect the account(s), create a status and activate it. */
	status = purple_savedstatus_new(NULL, PURPLE_STATUS_AVAILABLE);
	purple_savedstatus_activate(status);

	purple_account_set_status(info->prplAccount, "available", TRUE, NULL);
}

void close_purple(struct IRISData *info)
{
	// XYZZY - needed?
	purple_core_quit();
}
