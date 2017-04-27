/**
 * -*- linux-c-mode -*-
 *
 *
 */

#define _GNU_SOURCE
#include <getopt.h>
#include <signal.h>

#include "internal.h"

void show_version()
{
	g_printf("Version: %s\n", VERSION_STR);
}

void show_usage()
{
	g_printf("Usage: \n");
}


int main(int argc, char *argv[])
{
	int opt;
	char *opt_config_dir_arg = NULL;
	GMainLoop *loop;
	struct IRISData info = { .verbosity = DEFAULT_VERBOSITY };
	
	/*
	GOptionEntry opt_entries[] = {
		{ "replace", 'r', 0, G_OPTION_ARG_NONE, &opt_replace, "Replace existing name if possible", NULL },
		{ "allow-replacement", 'a', 0, G_OPTION_ARG_NONE, &opt_allow_replacement, "Allow replacement", NULL },
		{ "name", 'n', 0, G_OPTION_ARG_STRING, &opt_name, "Name to acquire", NULL },
		{ NULL}
	};
	*/
    
	struct option long_options[] = {
		{"config",   required_argument, NULL, 'c'},
		{"debug",    no_argument,       NULL, 'd'},
		{"quiet",    no_argument,	NULL, 'q'},
		{"help",     no_argument,       NULL, 'h'},
		{"version",  no_argument,       NULL, 'v'},
		{0, 0, 0, 0}
	};

#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

	g_set_prgname(PROG_NAME);
	g_set_application_name(PROG_NAME);

	/* scan command-line options */
	opterr = 1;
	while ((opt = getopt_long(argc, argv, "c:dqhn::v",
				  long_options, NULL)) != -1) {
		switch (opt) {
		case 'c':	/* config dir */
			g_free(opt_config_dir_arg);
			opt_config_dir_arg = g_strdup(optarg);
			break;
		case 'd':	/* debug */
			++info.verbosity;
			break;
		case 'q':
			--info.verbosity;
			break;
		case 'v':	/* version */
			show_version();
			break;
		case 'h':	/* help */
		case '?':	/* show terse help */
		default:
			show_usage();
			return 0;
			break;
		}
	}

	init_purple(&info, opt_config_dir_arg);
	init_gdbus(&info);
	
	// Start Loop
	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	close_gdbus(&info);
	return 0;
}
