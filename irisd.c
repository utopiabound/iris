/***
 * -*- linux-c  -*-
 * Copyright 2017 Nathaniel Clark <Nathaniel.Clark@misrule.us>
 * All Rights Reserved
 */

#define _GNU_SOURCE
#include <getopt.h>
#include <signal.h>
#include <locale.h>

#include "internal.h"

void show_version()
{
	g_printf("Version: %s\n", VERSION_STR);
}

void show_usage()
{
	g_printf("Usage: %s\n", PROG_NAME);
}


int main(int argc, char *argv[])
{
	int opt;
	char *opt_config_dir_arg = NULL;
	struct IRISData info = { .flags = IRIS_FLAG_NONE };
	
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
		{"system",   no_argument,	NULL, 's'},
		{"debug",    no_argument,       NULL, 'd'},
		{"help",     no_argument,       NULL, 'h'},
		{"version",  no_argument,       NULL, 'v'},
		{0, 0, 0, 0}
	};

	setlocale(LC_ALL, "");
#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

	g_set_prgname(PROG_NAME);
	g_set_application_name(PROG_NAME);

	/* scan command-line options */
	opterr = 1;
	while ((opt = getopt_long(argc, argv, "c:dhsv",
				  long_options, NULL)) != -1) {
		switch (opt) {
		case 'c':	/* config dir */
			g_free(opt_config_dir_arg);
			opt_config_dir_arg = g_strdup(optarg);
			break;
		case 'd':	/* debug */
			info.flags |= IRIS_FLAG_DEBUG;
			break;
		case 's':
			info.flags |= IRIS_FLAG_SYSTEM;
			break;
		case 'v':	/* version */
			show_version();
			return 0;
			break;
		case 'h':	/* help */
		case '?':	/* show terse help */
		default:
			show_usage();
			return 0;
			break;
		}
	}

	info.loop = g_main_loop_new(NULL, FALSE);

	init_purple(&info, opt_config_dir_arg);
	init_gdbus(&info);
	
	// Start Loop
	g_main_loop_run(info.loop);

	close_purple(&info);
	close_gdbus(&info);
	return 0;
}
