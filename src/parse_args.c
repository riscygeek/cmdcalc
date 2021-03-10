#include <stddef.h>
#include <getopt.h>
#include <stdio.h>
#include "strint.h"
#include "cmdcalc.h"

struct cmdline_args parse_args(int argc, char* argv[]) {
	static const char* short_options = "f:hD:";
	static const struct option long_options[] = {
		{ "file",   required_argument, NULL, 'f' },
		{ "version", no_argument, NULL, 0 },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};
	struct cmdline_args args = { 0 };
	int option_index = 0;
	int c;
	args.exitcode = -1;
	while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		const int this_option_optind = optind ? optind : 1;
		switch (c) {
		case 0:
			switch (option_index) {
			case 0: // file
				args.filename = optarg;
				break;
			case 1: // version
				printf("cmdcacc v%s by BenjamincStürz\n", CMDCALC_VERSION);
				puts("GitHub: https://github.com/Benni3D/cmdcalc");
				args.exitcode = 0;
				return args;
			case 2: // help
				print_help();
				args.exitcode = 0;
				return args;
			default:
				args.exitcode = 1;
				return args;
			}
			break;
		case 'f':
			args.filename = optarg;
			break;
		case 'h':
			print_help();
			args.exitcode = 0;
			return args;
		case 'D':
			printf("Pass-in Variables are currently not supported\n");
			break;
		case '?': return args.exitcode = 1, args;
		default:
			printf("Warning: getopt returned character code 0%o\n", c);
			break;
		}
	}
	char* expr = NULL;
	while (optind < argc) {
		const char* str = argv[optind];
		for (size_t i = 0; str[i]; ++i)
			buf_push(expr, str[i]);
		buf_push(expr, ' ');
		++optind;
	}
	if (expr) {
		buf_push(expr, '\0');
		args.expression = strint(expr);
		buf_free(expr);
	}
	else args.expression = NULL;
	return args;
}

void print_help(void) {
	// TODO: Implement Help Page
	printf("cmdcalc %s by Benjamin Stürz\n", CMDCALC_VERSION);
	puts("GitHub: https://github.com/Benni3D/cmdcalc");
	puts("List all functions: funcs");
	puts("List all variables: vars");
}
