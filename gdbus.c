/***
 * -*- linux-c  -*-
 * Copyright 2017 Nathaniel Clark <Nathaniel.Clark@misrule.us>
 * All Rights Reserved
 */

#include "internal.h"



/* Introspection data for the service we are exporting */
static const gchar introspection_xml[] =
  "<node>"
  "  <interface name='net.utopiabound.dbus.iris'>"
  "    <method name='TellAll'>"
  "      <arg type='s' name='message' direction='in'/>"
  "    </method>"
  "    <method name='ListAll'>"
  "      <arg type='as' name='message' direction='out'/>"
  "    </method>"
  "    <method name='Quit'>"
  "    </method>"
  "    <property type='s' name='Version' access='read'/>"
  "  </interface>"
  "</node>";

static void listall(gpointer data, gpointer user_data)
{
	PurpleBuddy *buddy = data;
	GVariantBuilder *builder = user_data;
	gchar buf[30];

	g_snprintf(buf, sizeof(buf), "%s (%s)",
		  buddy->name,
		  buddy->server_alias);

	g_variant_builder_add(builder, "s", buf);
}
static void sendeach(gpointer data, gpointer user_data) {
	PurpleBuddy *buddy = data;
	const gchar *msg = user_data;
	iris_send_message(buddy, msg);
}

static void handle_method_call(GDBusConnection       *connection,
			const gchar           *sender,
			const gchar           *object_path,
			const gchar           *interface_name,
			const gchar           *method_name,
			GVariant              *parameters,
			GDBusMethodInvocation *invocation,
			gpointer               user_data)
{
	struct IRISData *info = user_data;
	GSList *list;

	g_debug("%s:%s call %s %s.%s(%s)\n",__func__,
		  sender, object_path, interface_name, method_name,
		  g_variant_get_type_string(parameters));

	if (g_strcmp0(method_name, "TellAll") == 0) {
		const gchar* msg;
		GVariant *sub;
		sub = g_variant_get_child_value(parameters, 0);
		msg = g_variant_get_string(sub, NULL);

		g_debug("Will Tell World: %s", msg);

		// list all buddies
		list = purple_find_buddies(info->prplAccount, NULL);
		if (list != NULL) {
			g_slist_foreach(list, sendeach, (gpointer)msg);
			g_slist_free(list);
		}

		g_dbus_method_invocation_return_value(invocation, NULL);
		g_variant_unref(sub);

	} else if (g_strcmp0(method_name, "ListAll") == 0) {
		GVariant *ret = NULL;
		GVariantBuilder builder;

		// list all buddies
		list = purple_find_buddies(info->prplAccount, NULL);
		if (list != NULL) {
			g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));

			g_slist_foreach(list, listall, &builder);

			ret = g_variant_builder_end(&builder);

			g_slist_free(list);
		}

		// Encase string array in tuple
		g_variant_builder_init(&builder, G_VARIANT_TYPE("(as)"));
		g_variant_builder_add(&builder, "@as", ret);
		ret = g_variant_builder_end(&builder);

		g_dbus_method_invocation_return_value(invocation, ret);

	} else if (g_strcmp0(method_name, "Quit") == 0) {
		g_dbus_method_invocation_return_value(invocation, NULL);

		if (g_main_loop_is_running(info->loop))
			g_main_loop_quit(info->loop);
	}
}

GVariant *handle_get_property (GDBusConnection       *connection,
			       const gchar           *sender,
			       const gchar           *object_path,
			       const gchar           *interface_name,
			       const gchar           *property_name,
			       GError               **error,
			       gpointer               user_data)
{
	GVariant *ret = NULL;
	g_debug("%s:%s get %s %s.%s\n",__func__,
		  sender, object_path, interface_name, property_name);
	if (g_strcmp0(property_name, "Version") == 0)
		ret = g_variant_new_string(VERSION_STR);

	return ret;
}

static gboolean
handle_set_property (GDBusConnection  *connection,
		     const gchar      *sender,
		     const gchar      *object_path,
		     const gchar      *interface_name,
		     const gchar      *property_name,
		     GVariant         *value,
		     GError          **error,
		     gpointer          user_data)
{
	// NO WRITABLE PROPERTIES
	g_error("Trying to set %s %s.%s, but should be read-only\n",
		object_path, interface_name, property_name);
	return FALSE;
}



static const GDBusInterfaceVTable interface_vtable =
{
	handle_method_call,
	handle_get_property,
	handle_set_property
};

/* Bus Call-backs */
static void
on_bus_acquired (GDBusConnection *connection,
		 const gchar     *name,
		 gpointer         user_data)
{
	guint registration_id;
	struct IRISData *info = user_data;

	g_debug("%s:Bus Acquired %s\n",__func__, name);

	registration_id = g_dbus_connection_register_object(connection,
		"/",
		info->introspect->interfaces[0],
		&interface_vtable,
		user_data,
		NULL,  /* user_data_free_func */
		NULL); /* GError** */
	g_assert (registration_id > 0);
}

static void
on_name_acquired (GDBusConnection *connection,
		  const gchar     *name,
		  gpointer         user_data)
{
	g_debug("Acquired the name %s on the session bus\n", name);
}

static void
on_name_lost (GDBusConnection *connection,
	      const gchar     *name,
	      gpointer         user_data)
{
	g_message("Lost the name %s on the session bus\n", name);
}

static void log_dummy(const gchar *log_domain,
                     GLogLevelFlags log_level,
                     const gchar *message,
                     gpointer user_data )

{
	/* Dummy does nothing */
	return;
}

void init_gdbus(struct IRISData *info) {
	GBusNameOwnerFlags flags = G_BUS_NAME_OWNER_FLAGS_NONE;

	/*
	gboolean opt_replace;
	gboolean opt_allow_replacement;

	if (opt_replace)
		flags |= G_BUS_NAME_OWNER_FLAGS_REPLACE;
	if (opt_allow_replacement)
		flags |= G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT;
	*/
	info->introspect = g_dbus_node_info_new_for_xml(introspection_xml, NULL);

	info->gdbusID = g_bus_own_name (G_BUS_TYPE_SESSION,
					DBUS_NAME,
					flags,
					on_bus_acquired,
					on_name_acquired,
					on_name_lost,
					info,
					NULL);

	/* setup debug/logging
	 *  (relative to DEFAULT_VERBOSITY)
	 * -1 (or less)		Only WARN and CRIT
	 * ==			Default MASK
	 * +1			INFO and above
	 * +2			DEBUG and above
	 */
	g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MASK, log_dummy, NULL);
	if (info->verbosity > DEFAULT_VERBOSITY)
		g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, g_log_default_handler, NULL);

	else if (info->verbosity > DEFAULT_VERBOSITY+1)
		g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, g_log_default_handler, NULL);

	else if (info->verbosity < DEFAULT_VERBOSITY)
		g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING|G_LOG_LEVEL_CRITICAL, g_log_default_handler, NULL);
	else
		g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MASK, g_log_default_handler, NULL);
}

void close_gdbus(struct IRISData *info) {
	g_bus_unown_name(info->gdbusID);
}
