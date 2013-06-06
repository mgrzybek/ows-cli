/**
 * Project: ows-cli: a shell client for Open Workload Scheduler
 * File name: libcli.c
 * Description: implements the cli library (based on libcli)
 *
 * @author Mathieu Grzybek on 2013-06-06
 * @author David Parrish (libcli - LGPL 2.1: https://github.com/dparrish/libcli)
 * @copyright 2013 Mathieu Grzybek. All rights reserved.
 * @version $Id: code-gpl-license.txt,v 1.2 2004/05/04 13:19:30 garry Exp $
 *
 * @see The GNU Public License (GPL) version 3 or higher
 *
 *
 * ows-cli is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#if !defined(__APPLE__) && !defined(__FreeBSD__)
#include <malloc.h>
#endif
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#ifndef WIN32
#include <regex.h>
#endif
#include "libcli.h"

// vim:sw=4 tw=120 et

#ifdef __GNUC__
# define UNUSED(d) d __attribute__ ((unused))
#else
# define UNUSED(d) d
#endif

#define MATCH_REGEX     1
#define MATCH_INVERT    2

#ifdef WIN32
/*
 * Stupid windows has multiple namespaces for filedescriptors, with different
 * read/write functions required for each ..
 */
int read(int fd, void *buf, unsigned int count) {
	return recv(fd, buf, count, 0);
}

int write(int fd, const void *buf, unsigned int count) {
	return send(fd, buf, count, 0);
}

int vasprintf(char **strp, const char *fmt, va_list args) {
	int size;

	size = vsnprintf(NULL, 0, fmt, args);
	if ((*strp = malloc(size + 1)) == NULL) {
		return -1;
	}

	size = vsnprintf(*strp, size + 1, fmt, args);
	return size;
}

int asprintf(char **strp, const char *fmt, ...) {
	va_list args;
	int size;

	va_start(args, fmt);
	size = vasprintf(strp, fmt, args);

	va_end(args);
	return size;
}

int fprintf(FILE *stream, const char *fmt, ...) {
	va_list args;
	int size;
	char *buf;

	va_start(args, fmt);
	size = vasprintf(&buf, fmt, args);
	if (size < 0) {
		goto out;
	}
	size = write(stream->_file, buf, size);
	free(buf);

out:
	va_end(args);
	return size;
}

/*
 * Dummy definitions to allow compilation on Windows
 */
int regex_dummy() {return 0;};
#define regfree(...) regex_dummy()
#define regexec(...) regex_dummy()
#define regcomp(...) regex_dummy()
#define regex_t int
#define REG_NOSUB       0
#define REG_EXTENDED    0
#define REG_ICASE       0
#endif

enum cli_states {
	STATE_LOGIN,
	STATE_PASSWORD,
	STATE_NORMAL,
	STATE_ENABLE_PASSWORD,
	STATE_ENABLE
};

struct unp {
	char *username;
	char *password;
	struct unp *next;
};

struct cli_filter_cmds
{
	const char *cmd;
	const char *help;
};

/* free and zero (to avoid double-free) */
#define free_z(p) do { if (p) { free(p); (p) = 0; } } while (0)

int cli_match_filter_init(struct cli_def *cli, int argc, char **argv, struct cli_filter *filt);
int cli_range_filter_init(struct cli_def *cli, int argc, char **argv, struct cli_filter *filt);
int cli_count_filter_init(struct cli_def *cli, int argc, char **argv, struct cli_filter *filt);
int cli_match_filter(struct cli_def *cli, const char *string, void *data);
int cli_range_filter(struct cli_def *cli, const char *string, void *data);
int cli_count_filter(struct cli_def *cli, const char *string, void *data);

static struct cli_filter_cmds filter_cmds[] =
{
	{ "begin",   "Begin with lines that match" },
	{ "between", "Between lines that match" },
	{ "count",   "Count of lines"   },
	{ "exclude", "Exclude lines that match" },
	{ "include", "Include lines that match" },
	{ "grep",    "Include lines that match regex (options: -v, -i, -e)" },
	{ "egrep",   "Include lines that match extended regex" },
	{ NULL, NULL}
};
/*
static ssize_t _write(int fd, const void *buf, size_t count)
{
	size_t written = 0;
	ssize_t thisTime =0;
	while (count != written)
	{
		thisTime = write(fd, (char*)buf + written, count - written);
		if (thisTime == -1)
		{
			if (errno == EINTR)
				continue;
			else
				return -1;
		}
		written += thisTime;
	}
	return written;
}
*/
char *cli_command_name(struct cli_def *cli, struct cli_command *command)
{
	char *name = cli->commandname;
	char *o;

	if (name) free(name);
	if (!(name = calloc(1, 1)))
		return NULL;

	while (command)
	{
		o = name;
		if (asprintf(&name, "%s%s%s", command->command, *o ? " " : "", o) == -1)
		{
			fprintf(stderr, "Couldn't allocate memory for command_name: %s", strerror(errno));
			free(o);
			return NULL;
		}
		command = command->parent;
		free(o);
	}
	cli->commandname = name;
	return name;
}

void cli_set_auth_callback(struct cli_def *cli, int (*auth_callback)(const char *, const char *))
{
	cli->auth_callback = auth_callback;
}

void cli_set_enable_callback(struct cli_def *cli, int (*enable_callback)(const char *))
{
	cli->enable_callback = enable_callback;
}

void cli_allow_user(struct cli_def *cli, const char *username, const char *password)
{
	struct unp *u, *n;
	if (!(n = malloc(sizeof(struct unp))))
	{
		fprintf(stderr, "Couldn't allocate memory for user: %s", strerror(errno));
		return;
	}
	if (!(n->username = strdup(username)))
	{
		fprintf(stderr, "Couldn't allocate memory for username: %s", strerror(errno));
		free(n);
		return;
	}
	if (!(n->password = strdup(password)))
	{
		fprintf(stderr, "Couldn't allocate memory for password: %s", strerror(errno));
		free(n->username);
		free(n);
		return;
	}
	n->next = NULL;

	if (!cli->users)
	{
		cli->users = n;
	}
	else
	{
		for (u = cli->users; u && u->next; u = u->next);
		if (u) u->next = n;
	}
}

void cli_allow_enable(struct cli_def *cli, const char *password)
{
	free_z(cli->enable_password);
	if (!(cli->enable_password = strdup(password)))
	{
		fprintf(stderr, "Couldn't allocate memory for enable password: %s", strerror(errno));
	}
}

void cli_deny_user(struct cli_def *cli, const char *username)
{
	struct unp *u, *p = NULL;
	if (!cli->users) return;
	for (u = cli->users; u; u = u->next)
	{
		if (strcmp(username, u->username) == 0)
		{
			if (p)
				p->next = u->next;
			else
				cli->users = u->next;
			free(u->username);
			free(u->password);
			free(u);
			break;
		}
		p = u;
	}
}

void cli_set_hostname(struct cli_def *cli, const char *hostname)
{
	free_z(cli->hostname);
	if (hostname && *hostname)
		cli->hostname = strdup(hostname);
}

void cli_set_promptchar(struct cli_def *cli, const char *promptchar)
{
	free_z(cli->promptchar);
	cli->promptchar = strdup(promptchar);
}

static int cli_build_shortest(struct cli_def *cli, struct cli_command *commands)
{
	struct cli_command *c, *p;
	char *cp, *pp;
	unsigned len;

	for (c = commands; c; c = c->next)
	{
		c->unique_len = strlen(c->command);
		if ((c->mode != MODE_ANY && c->mode != cli->mode) || c->privilege > cli->privilege)
			continue;

		c->unique_len = 1;
		for (p = commands; p; p = p->next)
		{
			if (c == p)
				continue;

			if ((p->mode != MODE_ANY && p->mode != cli->mode) || p->privilege > cli->privilege)
				continue;

			cp = c->command;
			pp = p->command;
			len = 1;

			while (*cp && *pp && *cp++ == *pp++)
				len++;

			if (len > c->unique_len)
				c->unique_len = len;
		}

		if (c->children)
			cli_build_shortest(cli, c->children);
	}

	return CLI_OK;
}

int cli_set_privilege(struct cli_def *cli, int priv)
{
	int old = cli->privilege;
	cli->privilege = priv;

	if (priv != old)
	{
		if ( cli->promptchar == NULL ) {
			cli->promptchar = strdup((priv == PRIVILEGE_PRIVILEGED)?"# ":"> ");
		} else {
			if ( priv == PRIVILEGE_PRIVILEGED ) {
				cli->promptchar[strlen(cli->promptchar) - 2] = '#';
			} else {
				cli->promptchar[strlen(cli->promptchar) - 2] = '>';
			}
		}
	}

	return old;
}

void cli_set_modestring(struct cli_def *cli, const char *modestring)
{
	free_z(cli->modestring);
	if (modestring)
		cli->modestring = strdup(modestring);
}

int cli_set_configmode(struct cli_def *cli, int mode, const char *config_desc)
{
	int old = cli->mode;
	cli->mode = mode;

	if (mode != old)
	{
		if (!cli->mode)
		{
			// Not config mode
			cli_set_modestring(cli, NULL);
		}
		else if (config_desc && *config_desc)
		{
			char string[64];
			snprintf(string, sizeof(string), "(config-%s)", config_desc);
			cli_set_modestring(cli, string);
		}
		else
		{
			cli_set_modestring(cli, "(config)");
		}

		cli_build_shortest(cli, cli->commands);
	}

	return old;
}

struct cli_command *cli_register_command(struct cli_def *cli, struct cli_command *parent, const char *command, int
	(*callback)(struct cli_def *cli, const char *, char **, int), int privilege,
	int mode, const char *help)
{
	struct cli_command *c, *p;

	if (!command) return NULL;
	if (!(c = calloc(sizeof(struct cli_command), 1))) return NULL;

	c->callback = callback;
	c->next = NULL;
	if (!(c->command = strdup(command)))
		return NULL;
	c->parent = parent;
	c->privilege = privilege;
	c->mode = mode;
	if (help && !(c->help = strdup(help)))
		return NULL;

	if (parent)
	{
		if (!parent->children)
		{
			parent->children = c;
		}
		else
		{
			for (p = parent->children; p && p->next; p = p->next);
			if (p) p->next = c;
		}
	}
	else
	{
		if (!cli->commands)
		{
			cli->commands = c;
		}
		else
		{
			for (p = cli->commands; p && p->next; p = p->next);
			if (p) p->next = c;
		}
	}
	return c;
}

static void cli_free_command(struct cli_command *cmd)
{
	struct cli_command *c, *p;

	for (c = cmd->children; c;)
	{
		p = c->next;
		cli_free_command(c);
		c = p;
	}

	free(cmd->command);
	if (cmd->help) free(cmd->help);
	free(cmd);
}

int cli_unregister_command(struct cli_def *cli, const char *command)
{
	struct cli_command *c, *p = NULL;

	if (!command) return -1;
	if (!cli->commands) return CLI_OK;

	for (c = cli->commands; c; c = c->next)
	{
		if (strcmp(c->command, command) == 0)
		{
			if (p)
				p->next = c->next;
			else
				cli->commands = c->next;

			cli_free_command(c);
			return CLI_OK;
		}
		p = c;
	}

	return CLI_OK;
}

int cli_show_help(struct cli_def *cli, struct cli_command *c)
{
	struct cli_command *p;

	for (p = c; p; p = p->next)
	{
		if (p->command && p->callback && cli->privilege >= p->privilege &&
				(p->mode == cli->mode || p->mode == MODE_ANY))
		{
			printf("  %-20s %s\n", cli_command_name(cli, p), (p->help != NULL ? p->help : ""));
		}

		if (p->children)
			cli_show_help(cli, p->children);
	}

	return CLI_OK;
}

int cli_int_enable(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	if (cli->privilege == PRIVILEGE_PRIVILEGED)
		return CLI_OK;

	if (!cli->enable_password && !cli->enable_callback)
	{
		/* no password required, set privilege immediately */
		cli_set_privilege(cli, PRIVILEGE_PRIVILEGED);
	}
	else
	{
		/* require password entry */
		cli->state = STATE_ENABLE_PASSWORD;
	}

	return CLI_OK;
}

int cli_int_disable(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
	cli_set_configmode(cli, MODE_EXEC, NULL);
	return CLI_OK;
}

int cli_int_help(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	//cli_error("\nCommands available:");
	printf("\nCommands available:\n");
	cli_show_help(cli, cli->commands);
	return CLI_OK;
}

int cli_int_history(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	int i;

	//cli_error(cli, "\nCommand history:");
	printf("\nCommand history:\n");
	for (i = 0; i < MAX_HISTORY; i++)
	{
		if (cli->history[i])
			printf( "%3d. %s", i, cli->history[i]);
	}

	return CLI_OK;
}

int cli_int_quit(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
	cli_set_configmode(cli, MODE_DISCONNECTED, NULL);
	return CLI_QUIT;
}

int cli_int_exit(struct cli_def *cli, const char *command, char *argv[], int argc)
{
	if (cli->mode == MODE_EXEC)
		return cli_int_quit(cli, command, argv, argc);

	if (cli->mode > MODE_CONFIG)
		cli_set_configmode(cli, MODE_CONFIG, NULL);
	else
		cli_set_configmode(cli, MODE_EXEC, NULL);

	cli->service = NULL;
	return CLI_OK;
}

int cli_int_idle_timeout(struct cli_def *cli)
{
	//cli_print(cli, "Idle timeout");
	printf("Idle timeout");
	return CLI_QUIT;
}

int cli_int_configure_terminal(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	cli_set_configmode(cli, MODE_CONFIG, NULL);
	return CLI_OK;
}

struct cli_def *cli_init()
{
	struct cli_def *cli;
	struct cli_command *c;

	if (!(cli = calloc(sizeof(struct cli_def), 1)))
		return 0;

	cli->buf_size = 1024;
	if (!(cli->buffer = calloc(cli->buf_size, 1)))
	{
		free_z(cli);
		return 0;
	}

	cli_register_command(cli, 0, "help", cli_int_help, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Show available commands");
	cli_register_command(cli, 0, "quit", cli_int_quit, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Disconnect");
	cli_register_command(cli, 0, "history", cli_int_history, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Show a list of previously run commands");
	cli_register_command(cli, 0, "enable", cli_int_enable, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Turn on privileged commands");
	cli_register_command(cli, 0, "disable", cli_int_disable, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, "Turn off privileged commands");

	c = cli_register_command(cli, 0, "configure", 0, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, "Enter configuration mode");
	cli_register_command(cli, c, "terminal", cli_int_configure_terminal, PRIVILEGE_PRIVILEGED, MODE_EXEC,
			"Configure from the terminal");

	cli->privilege = cli->mode = -1;
	cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
	cli_set_configmode(cli, MODE_DISCONNECTED, 0);

	// Set connection state
	cli->connected = MODE_DISCONNECTED;

	// Default to 1 second timeout intervals
	cli->timeout_tm.tv_sec = 1;
	cli->timeout_tm.tv_usec = 0;

	// Set default idle timeout callback, but no timeout
	cli_set_idle_timeout_callback(cli, 0, cli_int_idle_timeout);
	return cli;
}

void cli_unregister_all(struct cli_def *cli, struct cli_command *command)
{
	struct cli_command *c, *p = NULL;

	if (!command) command = cli->commands;
	if (!command) return;

	for (c = command; c; )
	{
		p = c->next;

		// Unregister all child commands
		if (c->children)
			cli_unregister_all(cli, c->children);

		if (c->command) free(c->command);
		if (c->help) free(c->help);
		free(c);

		c = p;
	}
}

int cli_done(struct cli_def *cli)
{
	if ( cli == NULL )
		return CLI_ERROR;

	struct unp *u = cli->users, *n;

	if (!cli) return CLI_OK;
	//cli_free_history(cli);

	// Free all users
	while (u)
	{
		if (u->username) free(u->username);
		if (u->password) free(u->password);
		n = u->next;
		free(u);
		u = n;
	}

	/* free all commands */
	cli_unregister_all(cli, 0);

	free_z(cli->commandname);
	free_z(cli->modestring);
	free_z(cli->promptchar);
	free_z(cli->hostname);
	free_z(cli->buffer);
	free_z(cli);

	return CLI_OK;
}
/*
static int cli_add_history(struct cli_def *cli, const char *cmd)
{
	int i;
	for (i = 0; i < MAX_HISTORY; i++)
	{
		if (!cli->history[i])
		{
			if (i == 0 || strcasecmp(cli->history[i-1], cmd))
				if (!(cli->history[i] = strdup(cmd)))
					return CLI_ERROR;
			return CLI_OK;
		}
	}
	// No space found, drop one off the beginning of the list
	free(cli->history[0]);
	for (i = 0; i < MAX_HISTORY-1; i++)
		cli->history[i] = cli->history[i+1];
	if (!(cli->history[MAX_HISTORY - 1] = strdup(cmd)))
		return CLI_ERROR;
	return CLI_OK;
}

void cli_free_history(struct cli_def *cli)
{
	int i;
	for (i = 0; i < MAX_HISTORY; i++)
	{
		if (cli->history[i])
			free_z(cli->history[i]);
	}
}
*/
static int cli_parse_line(const char *line, char *words[], int max_words)
{
	int nwords = 0;
	const char *p = line;
	const char *word_start = 0;
	int inquote = 0;

	while (*p)
	{
		if (!isspace(*p))
		{
			word_start = p;
			break;
		}
		p++;
	}

	while (nwords < max_words - 1)
	{
		if (!*p || *p == inquote || (word_start && !inquote && (isspace(*p) || *p == '|')))
		{
			if (word_start)
			{
				int len = p - word_start;

				memcpy(words[nwords] = malloc(len + 1), word_start, len);
				words[nwords++][len] = 0;
			}

			if (!*p)
				break;

			if (inquote)
				p++; /* skip over trailing quote */

			inquote = 0;
			word_start = 0;
		}
		else if (*p == '"' || *p == '\'')
		{
			inquote = *p++;
			word_start = p;
		}
		else
		{
			if (!word_start)
			{
				if (*p == '|')
				{
					if (!(words[nwords++] = strdup("|")))
						return 0;
				}
				else if (!isspace(*p))
					word_start = p;
			}

			p++;
		}
	}

	return nwords;
}

static char *join_words(int argc, char **argv)
{
	char *p;
	int len = 0;
	int i;

	for (i = 0; i < argc; i++)
	{
		if (i)
			len += 1;

		len += strlen(argv[i]);
	}

	p = malloc(len + 1);
	p[0] = 0;

	for (i = 0; i < argc; i++)
	{
		if (i)
			strcat(p, " ");

		strcat(p, argv[i]);
	}

	return p;
}

static int cli_find_command(struct cli_def *cli, struct cli_command *commands, int num_words, char *words[],
		int start_word, int filters[])
{
	struct cli_command *c, *again_config = NULL, *again_any = NULL;
	int c_words = num_words;

	if (filters[0])
		c_words = filters[0];

	// Deal with ? for help
	if (!words[start_word])
		return CLI_ERROR;

	if (words[start_word][strlen(words[start_word]) - 1] == '?')
	{
		int l = strlen(words[start_word])-1;

		if (commands->parent && commands->parent->callback)
			printf( "%-20s %s\n", cli_command_name(cli, commands->parent),
					(commands->parent->help != NULL ? commands->parent->help : ""));

		for (c = commands; c; c = c->next)
		{
			if (strncasecmp(c->command, words[start_word], l) == 0
					&& (c->callback || c->children)
					&& cli->privilege >= c->privilege
					&& (c->mode == cli->mode || c->mode == MODE_ANY))
				printf( "  %-20s %s\n", c->command, (c->help != NULL ? c->help : ""));
		}

		return CLI_OK;
	}

	for (c = commands; c; c = c->next)
	{
		if (cli->privilege < c->privilege)
			continue;

		if (strncasecmp(c->command, words[start_word], c->unique_len))
			continue;

		if (strncasecmp(c->command, words[start_word], strlen(words[start_word])))
			continue;

AGAIN:
		if (c->mode == cli->mode || (c->mode == MODE_ANY && again_any != NULL) || (c->mode == MODE_DISCONNECTED && cli->mode == MODE_EXEC))
		{
			int rc = CLI_OK;
			int f;
			struct cli_filter **filt = &cli->filters;

			// Found a word!
			if (!c->children)
			{
				// Last word
				if (!c->callback)
				{
					printf( "No callback for \"%s\"", cli_command_name(cli, c));
					return CLI_ERROR;
				}
			}
			else
			{
				if (start_word == c_words - 1)
				{
					if (c->callback)
						goto CORRECT_CHECKS;

					printf( "Incomplete command");
					return CLI_ERROR;
				}
				rc = cli_find_command(cli, c->children, num_words, words, start_word + 1, filters);
				if (rc == CLI_ERROR_ARG)
				{
					if (c->callback)
					{
						rc = CLI_OK;
						goto CORRECT_CHECKS;
					}
					else
					{
						printf( "Invalid %s \"%s\"\n", commands->parent ? "argument" : "command",
								words[start_word]);
					}
				}
				return rc;
			}

			if (!c->callback)
			{
				printf( "Internal server error processing \"%s\"\n", cli_command_name(cli, c));
				return CLI_ERROR;
			}

CORRECT_CHECKS:
			for (f = 0; rc == CLI_OK && filters[f]; f++)
			{
				int n = num_words;
				char **argv;
				int argc;
				int len;

				if (filters[f+1])
					n = filters[f+1];

				if (filters[f] == n - 1)
				{
					printf( "Missing filter");
					return CLI_ERROR;
				}

				argv = words + filters[f] + 1;
				argc = n - (filters[f] + 1);
				len = strlen(argv[0]);
				if (argv[argc - 1][strlen(argv[argc - 1]) - 1] == '?')
				{
					if (argc == 1)
					{
						int i;
						for (i = 0; filter_cmds[i].cmd; i++)
							printf( "  %-20s %s", filter_cmds[i].cmd, filter_cmds[i].help );
					}
					else
					{
						if (argv[0][0] != 'c') // count
							printf( "  WORD");

						if (argc > 2 || argv[0][0] == 'c') // count
							printf( "  <cr>");
					}

					return CLI_OK;
				}

				if (argv[0][0] == 'b' && len < 3) // [beg]in, [bet]ween
				{
					printf( "Ambiguous filter \"%s\" (begin, between)", argv[0]);
					return CLI_ERROR;
				}
				*filt = calloc(sizeof(struct cli_filter), 1);

				if (!strncmp("include", argv[0], len) || !strncmp("exclude", argv[0], len) ||
						!strncmp("grep", argv[0], len) || !strncmp("egrep", argv[0], len))
					rc = cli_match_filter_init(cli, argc, argv, *filt);
				else if (!strncmp("begin", argv[0], len) || !strncmp("between", argv[0], len))
					rc = cli_range_filter_init(cli, argc, argv, *filt);
				else if (!strncmp("count", argv[0], len))
					rc = cli_count_filter_init(cli, argc, argv, *filt);
				else
				{
					printf( "Invalid filter \"%s\"\n", argv[0]);
					rc = CLI_ERROR;
				}

				if (rc == CLI_OK)
				{
					filt = &(*filt)->next;
				}
				else
				{
					free(*filt);
					*filt = 0;
				}
			}

			if (rc == CLI_OK)
				rc = c->callback(cli, cli_command_name(cli, c), words + start_word + 1, c_words - start_word - 1);

			while (cli->filters)
			{
				struct cli_filter *filt = cli->filters;

				// call one last time to clean up
				filt->filter(cli, NULL, filt->data);
				cli->filters = filt->next;
				free(filt);
			}

			return rc;
		}
		else if (cli->mode > MODE_CONFIG && c->mode == MODE_CONFIG)
		{
			// command matched but from another mode,
			// remember it if we fail to find correct command
			again_config = c;
		}
		else if (c->mode == MODE_ANY)
		{
			// command matched but for any mode,
			// remember it if we fail to find correct command
			again_any = c;
		}
	}

	// drop out of config submode if we have matched command on MODE_CONFIG
	if (again_config)
	{
		c = again_config;
		cli_set_configmode(cli, MODE_CONFIG, NULL);
		goto AGAIN;
	}
	if (again_any)
	{
		c = again_any;
		goto AGAIN;
	}

	if (start_word == 0)
		printf( "Invalid %s \"%s\"\n", commands->parent ? "argument" : "command", words[start_word]);

	return CLI_ERROR_ARG;
}

int cli_run_command(struct cli_def *cli, const char *command)
{
	int r;
	unsigned int num_words, i, f;
	char *words[CLI_MAX_LINE_WORDS] = {0};
	int filters[CLI_MAX_LINE_WORDS] = {0};

	if (!command) return CLI_ERROR;
	while (isspace(*command))
		command++;

	if (!*command) return CLI_OK;

	num_words = cli_parse_line(command, words, CLI_MAX_LINE_WORDS);
	for (i = f = 0; i < num_words && f < CLI_MAX_LINE_WORDS - 1; i++)
	{
		if (words[i][0] == '|')
			filters[f++] = i;
	}

	filters[f] = 0;

	if (num_words)
		r = cli_find_command(cli, cli->commands, num_words, words, 0, filters);
	else
		r = CLI_ERROR;

	for (i = 0; i < num_words; i++)
		free(words[i]);

	if (r == CLI_QUIT)
		return r;

	return CLI_OK;
}

char** cli_get_completions(const char *command, int start, int stop)
{
	struct cli_command *c;
	struct cli_command *n;
	int num_words, save_words, i, k=0;
	char *words[CLI_MAX_LINE_WORDS] = {0};
	int filter = 0;
	int max_completions = 20;
	char**	completions = (char**)calloc(sizeof(char),max_completions);

	if (!command) return 0;
	while (isspace(*command))
		command++;

	save_words = num_words = cli_parse_line(command, words, sizeof(words)/sizeof(words[0]));
	if (!command[0] || command[strlen(command)-1] == ' ')
		num_words++;

	if (!num_words)
		goto out;

	for (i = 0; i < num_words; i++)
	{
		if (words[i] && words[i][0] == '|')
			filter = i;
	}

	if (filter) // complete filters
	{
		unsigned len = 0;

		if (filter < num_words - 1) // filter already completed
			goto out;

		if (filter == num_words - 1)
			len = strlen(words[num_words-1]);

		for (i = 0; filter_cmds[i].cmd && k < max_completions; i++)
		{
			if (!len || (len < strlen(filter_cmds[i].cmd) && !strncmp(filter_cmds[i].cmd, words[num_words - 1], len)))
				completions[k++] = (char *)filter_cmds[i].cmd;
		}

		completions[k] = NULL;
		goto out;
	}

	for (c = global_cli->commands, i = 0; c && i < num_words && k < max_completions; c = n)
	{
		n = c->next;

		if (global_cli->privilege < c->privilege)
			continue;

		if (c->mode != global_cli->mode && c->mode != MODE_ANY)
			continue;

		if (words[i] && strncasecmp(c->command, words[i], strlen(words[i])))
			continue;

		if (i < num_words - 1)
		{
			if (strlen(words[i]) < c->unique_len)
				continue;

			n = c->children;
			i++;
			continue;
		}

		completions[k++] = c->command;
	}

out:
	for (i = 0; i < save_words; i++)
		free(words[i]);

	return completions;
}
/*
static int cli_get_completions(struct cli_def *cli, const char *command, char **completions, int max_completions)
{
	struct cli_command *c;
	struct cli_command *n;
	int num_words, save_words, i, k=0;
	char *words[CLI_MAX_LINE_WORDS] = {0};
	int filter = 0;

	if (!command) return 0;
	while (isspace(*command))
		command++;

	save_words = num_words = cli_parse_line(command, words, sizeof(words)/sizeof(words[0]));
	if (!command[0] || command[strlen(command)-1] == ' ')
		num_words++;

	if (!num_words)
		goto out;

	for (i = 0; i < num_words; i++)
	{
		if (words[i] && words[i][0] == '|')
			filter = i;
	}

	if (filter) // complete filters
	{
		unsigned len = 0;

		if (filter < num_words - 1) // filter already completed
			goto out;

		if (filter == num_words - 1)
			len = strlen(words[num_words-1]);

		for (i = 0; filter_cmds[i].cmd && k < max_completions; i++)
		{
			if (!len || (len < strlen(filter_cmds[i].cmd) && !strncmp(filter_cmds[i].cmd, words[num_words - 1], len)))
				completions[k++] = (char *)filter_cmds[i].cmd;
		}

		completions[k] = NULL;
		goto out;
	}

	for (c = cli->commands, i = 0; c && i < num_words && k < max_completions; c = n)
	{
		n = c->next;

		if (cli->privilege < c->privilege)
			continue;

		if (c->mode != cli->mode && c->mode != MODE_ANY)
			continue;

		if (words[i] && strncasecmp(c->command, words[i], strlen(words[i])))
			continue;

		if (i < num_words - 1)
		{
			if (strlen(words[i]) < c->unique_len)
				continue;

			n = c->children;
			i++;
			continue;
		}

		completions[k++] = c->command;
	}

out:
	for (i = 0; i < save_words; i++)
		free(words[i]);

	return k;
}
*/
/*
static void cli_clear_line(int sockfd, char *cmd, int l, int cursor)
{
	int i;
	if (cursor < l)
	{
		for (i = 0; i < (l - cursor); i++)
			_write(sockfd, " ", 1);
	}
	for (i = 0; i < l; i++)
		cmd[i] = '\b';
	for (; i < l * 2; i++)
		cmd[i] = ' ';
	for (; i < l * 3; i++)
		cmd[i] = '\b';
	_write(sockfd, cmd, i);
	memset((char *)cmd, 0, i);
	l = cursor = 0;
}
*/
void cli_reprompt(struct cli_def *cli)
{
	if (!cli) return;
	cli->showprompt = 1;
}

void cli_regular(struct cli_def *cli, int (*callback)(struct cli_def *cli))
{
	if (!cli) return;
	cli->regular_callback = callback;
}

void cli_regular_interval(struct cli_def *cli, int seconds)
{
	if (seconds < 1) seconds = 1;
	cli->timeout_tm.tv_sec = seconds;
	cli->timeout_tm.tv_usec = 0;
}

#define DES_PREFIX "{crypt}"        /* to distinguish clear text from DES crypted */
#define MD5_PREFIX "$1$"
/*
static int pass_matches(const char *pass, const char *try)
{
	int des;
	if ((des = !strncasecmp(pass, DES_PREFIX, sizeof(DES_PREFIX)-1)))
		pass += sizeof(DES_PREFIX)-1;

#ifndef WIN32
	//
	//  TODO - find a small crypt(3) function for use on windows
	//
	if (des || !strncmp(pass, MD5_PREFIX, sizeof(MD5_PREFIX)-1))
		try = crypt(try, pass);
#endif

	return !strcmp(pass, try);
}
*/
//#define CTRL(c) (c - '@')
/*
static int show_prompt(struct cli_def *cli, int sockfd)
{
	int len = 0;

	if (cli->hostname)
		len += write(sockfd, cli->hostname, strlen(cli->hostname));

	if (cli->modestring)
		len += write(sockfd, cli->modestring, strlen(cli->modestring));

	return len + write(sockfd, cli->promptchar, strlen(cli->promptchar));
}
*/
int cli_loop(struct cli_def *cli) {
	char*	line;
	char*	s;

	if ( cli == NULL )
		return -1;

	global_cli = cli;

	//initialize_readline ();	/* Bind our completer. */

	for ( ;; )
	{
		line = readline (cli->promptchar);

		if (!line)
			break;

		s = line;

		if (*s) {
			add_history (s);
			if (cli_run_command(cli, s) == CLI_QUIT)
				break;
		}

		free (line);
	}
	return 0;
}

int cli_file(struct cli_def *cli, FILE *fh, int privilege, int mode)
{
	int oldpriv = cli_set_privilege(cli, privilege);
	int oldmode = cli_set_configmode(cli, mode, NULL);
	char buf[CLI_MAX_LINE_LENGTH];

	while (1)
	{
		char *p;
		char *cmd;
		char *end;

		if (fgets(buf, CLI_MAX_LINE_LENGTH - 1, fh) == NULL)
			break; /* end of file */

		if ((p = strpbrk(buf, "#\r\n")))
			*p = 0;

		cmd = buf;
		while (isspace(*cmd))
			cmd++;

		if (!*cmd)
			continue;

		for (p = end = cmd; *p; p++)
			if (!isspace(*p))
				end = p;

		*++end = 0;
		if (strcasecmp(cmd, "quit") == 0)
			break;

		if (cli_run_command(cli, cmd) == CLI_QUIT)
			break;
	}

	cli_set_privilege(cli, oldpriv);
	cli_set_configmode(cli, oldmode, NULL /* didn't save desc */);

	return CLI_OK;
}

static void _print(struct cli_def *cli, int print_mode, const char *format, va_list ap)
{
	va_list aq;
	int n;
	char *p;

	if (!cli) return; // sanity check

	while (1)
	{
		va_copy(aq, ap);
		if ((n = vsnprintf(cli->buffer, cli->buf_size, format, ap)) == -1)
			return;

		if ((unsigned)n >= cli->buf_size)
		{
			cli->buf_size = n + 1;
			cli->buffer = realloc(cli->buffer, cli->buf_size);
			if (!cli->buffer)
				return;
			va_end(ap);
			va_copy(ap, aq);
			continue;
		}
		break;
	}


	p = cli->buffer;
	do
	{
		char *next = strchr(p, '\n');
		struct cli_filter *f = (print_mode & PRINT_FILTERED) ? cli->filters : 0;
		int print = 1;

		if (next)
			*next++ = 0;
		else if (print_mode & PRINT_BUFFERED)
			break;

		while (print && f)
		{
			print = (f->filter(cli, p, f->data) == CLI_OK);
			f = f->next;
		}
		if (print)
		{
			if (cli->print_callback)
				cli->print_callback(cli, p);
			else if (cli->client)
				fprintf(cli->client, "%s\r\n", p);
		}

		p = next;
	} while (p);

	if (p && *p)
	{
		if (p != cli->buffer)
			memmove(cli->buffer, p, strlen(p));
	}
	else *cli->buffer = 0;
}

void cli_bufprint(struct cli_def *cli, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	_print(cli, PRINT_BUFFERED|PRINT_FILTERED, format, ap);
	va_end(ap);
}

void cli_vabufprint(struct cli_def *cli, const char *format, va_list ap)
{
	_print(cli, PRINT_BUFFERED, format, ap);
}
/*
void cli_print(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	printf(format, ap);
	va_end(ap);
}

void cli_error(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	printf(PRINT_PLAIN, format, ap);
	va_end(ap);
}
*/
struct cli_match_filter_state
{
	int flags;
	union {
		char *string;
		regex_t re;
	} match;
};

int cli_match_filter_init(struct cli_def *cli, int argc, char **argv, struct cli_filter *filt)
{
	struct cli_match_filter_state *state;
	int rflags;
	int i;
	char *p;

	if (argc < 2)
	{
		if (cli->client)
			fprintf(cli->client, "Match filter requires an argument\r\n");

		return CLI_ERROR;
	}

	filt->filter = cli_match_filter;
	filt->data = state = calloc(sizeof(struct cli_match_filter_state), 1);

	if (argv[0][0] == 'i' || (argv[0][0] == 'e' && argv[0][1] == 'x'))  // include/exclude
	{
		if (argv[0][0] == 'e')
			state->flags = MATCH_INVERT;

		state->match.string = join_words(argc-1, argv+1);
		return CLI_OK;
	}

#ifdef WIN32
	/*
	 * No regex functions in windows, so return an error
	 */
	return CLI_ERROR;
#endif

	state->flags = MATCH_REGEX;

	// grep/egrep
	rflags = REG_NOSUB;
	if (argv[0][0] == 'e') // egrep
		rflags |= REG_EXTENDED;

	i = 1;
	while (i < argc - 1 && argv[i][0] == '-' && argv[i][1])
	{
		int last = 0;
		p = &argv[i][1];

		if (strspn(p, "vie") != strlen(p))
			break;

		while (*p)
		{
			switch (*p++)
			{
				case 'v':
					state->flags |= MATCH_INVERT;
					break;

				case 'i':
					rflags |= REG_ICASE;
					break;

				case 'e':
					last++;
					break;
			}
		}

		i++;
		if (last)
			break;
	}

	p = join_words(argc-i, argv+i);
	if ((i = regcomp(&state->match.re, p, rflags)))
	{
		if (cli->client)
			fprintf(cli->client, "Invalid pattern \"%s\"\r\n", p);

		free_z(p);
		return CLI_ERROR;
	}

	free_z(p);
	return CLI_OK;
}

int cli_match_filter(UNUSED(struct cli_def *cli), const char *string, void *data)
{
	struct cli_match_filter_state *state = data;
	int r = CLI_ERROR;

	if (!string) // clean up
	{
		if (state->flags & MATCH_REGEX)
			regfree(&state->match.re);
		else
			free(state->match.string);

		free(state);
		return CLI_OK;
	}

	if (state->flags & MATCH_REGEX)
	{
		if (!regexec(&state->match.re, string, 0, NULL, 0))
			r = CLI_OK;
	}
	else
	{
		if (strstr(string, state->match.string))
			r = CLI_OK;
	}

	if (state->flags & MATCH_INVERT)
	{
		if (r == CLI_OK)
			r = CLI_ERROR;
		else
			r = CLI_OK;
	}

	return r;
}

struct cli_range_filter_state {
	int matched;
	char *from;
	char *to;
};

int cli_range_filter_init(struct cli_def *cli, int argc, char **argv, struct cli_filter *filt)
{
	struct cli_range_filter_state *state;
	char *from = 0;
	char *to = 0;

	if (!strncmp(argv[0], "bet", 3)) // between
	{
		if (argc < 3)
		{
			if (cli->client)
				fprintf(cli->client, "Between filter requires 2 arguments\r\n");

			return CLI_ERROR;
		}

		if (!(from = strdup(argv[1])))
			return CLI_ERROR;
		to = join_words(argc-2, argv+2);
	}
	else // begin
	{
		if (argc < 2)
		{
			if (cli->client)
				fprintf(cli->client, "Begin filter requires an argument\r\n");

			return CLI_ERROR;
		}

		from = join_words(argc-1, argv+1);
	}

	filt->filter = cli_range_filter;
	filt->data = state = calloc(sizeof(struct cli_range_filter_state), 1);

	state->from = from;
	state->to = to;

	return CLI_OK;
}

int cli_range_filter(UNUSED(struct cli_def *cli), const char *string, void *data)
{
	struct cli_range_filter_state *state = data;
	int r = CLI_ERROR;

	if (!string) // clean up
	{
		free_z(state->from);
		free_z(state->to);
		free_z(state);
		return CLI_OK;
	}

	if (!state->matched)
		state->matched = !!strstr(string, state->from);

	if (state->matched)
	{
		r = CLI_OK;
		if (state->to && strstr(string, state->to))
			state->matched = 0;
	}

	return r;
}

int cli_count_filter_init(struct cli_def *cli, int argc, UNUSED(char **argv), struct cli_filter *filt)
{
	if (argc > 1)
	{
		if (cli->client)
			fprintf(cli->client, "Count filter does not take arguments\r\n");

		return CLI_ERROR;
	}

	filt->filter = cli_count_filter;
	if (!(filt->data = calloc(sizeof(int), 1)))
		return CLI_ERROR;

	return CLI_OK;
}

int cli_count_filter(struct cli_def *cli, const char *string, void *data)
{
	int *count = data;

	if (!string) // clean up
	{
		// print count
		if (cli->client)
			fprintf(cli->client, "%d\r\n", *count);

		free(count);
		return CLI_OK;
	}

	while (isspace(*string))
		string++;

	if (*string)
		(*count)++;  // only count non-blank lines

	return CLI_ERROR; // no output
}

void cli_print_callback(struct cli_def *cli, void (*callback)(struct cli_def *, const char *))
{
	cli->print_callback = callback;
}

void cli_set_idle_timeout(struct cli_def *cli, unsigned int seconds)
{
	if (seconds < 1)
		seconds = 0;
	cli->idle_timeout = seconds;
	time(&cli->last_action);
}

void cli_set_idle_timeout_callback(struct cli_def *cli, unsigned int seconds, int (*callback)(struct cli_def *))
{
	cli_set_idle_timeout(cli, seconds);
	cli->idle_timeout_callback = callback;
}

void cli_set_context(struct cli_def *cli, void *context) {
	cli->user_context = context;
}

void *cli_get_context(struct cli_def *cli) {
	return cli->user_context;
}
