/**
 * Project: ows-cli: a shell client for Open Workload Scheduler
 * File name: main.h
 * Description: describes the main file
 *
 * @author Mathieu Grzybek on 2013-06-06
 * @copyright 2010 Mathieu Grzybek. All rights reserved.
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

#ifndef _MAIN_H_
#include <fstream>
#include <iostream>
#include <algorithm>

#include <boost/foreach.hpp>

#include "text_processing.h"
#include "libcli.h"
#include "printing.h"
#include "rpc_client.h"

/**
 * cli_add_commands
 *
 * Adds the user commands to the cli_def struct
 *
 * @arg	the cli_def struct
 *
 * @return	true on success
 */
bool	cli_add_commands(struct cli_def* cli);

/**
 * client
 *
 * This object represents the connection against the node. It needs to be global
 * to be seen by the "cmd_*" functions.
 */
Rpc_Client	client;
rpc::t_node	local_node;
rpc::t_node	target_node;

#ifdef __GNUC__
#define UNUSED(d) d __attribute__ ((unused))
#else
#define UNUSED(d) d
#endif

/**
 * cmd_get_nodes
 *
 * Implements the get_nodes RPC call
 *
 * @return	CLI_OK or CLI_ERROR
 */
int	cmd_get_nodes(UNUSED(struct cli_def *cli), UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc));

/**
 * cmd_add_node
 *
 * Implements the add_node RPC call
 *
 * @arg	argv	the arguments
 * @arg argc	the number of arguments
 * @return	CLI_OK or CLI_ERROR
 */
int	cmd_add_node(UNUSED(struct cli_def *cli), UNUSED(const char *command), char *argv[], int argc);

/**
 * cmd_remove_node
 *
 * Implements the remove_node RPC call
 *
 * @arg	argv	the arguments
 * @arg argc	the number of arguments
 * @return	CLI_OK or CLI_ERROR
 */
int	cmd_remove_node(UNUSED(struct cli_def *cli), UNUSED(const char *command), char *argv[], int argc);

/**
 * cmd_remove_job
 *
 * Implements the remove_node RPC call
 *
 * @arg	argv	the arguments
 * @arg argc	the number of arguments
 * @return	CLI_OK or CLI_ERROR
 */
int	cmd_remove_job(UNUSED(struct cli_def *cli), UNUSED(const char *command), char *argv[], int argc);

/**
 * cmd_get_ready_jobs
 *
 * Implements the get_ready_jobs RPC call
 *
 * @return	CLI_OK or CLI_ERROR
 */
int	cmd_get_ready_jobs(UNUSED(struct cli_def *cli), UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc));

/**
 * cmd_get_jobs
 *
 * Implements the get_jobs RPC call
 *
 * @return	CLI_OK or CLI_ERROR
 */
int	cmd_get_jobs(UNUSED(struct cli_def *cli), UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc));

/**
 * cmd_connect
 *
 * Initializes the connection
 *
 * @arg	cli	the cli struct to update
 *
 * @return	CLI_OK or CLI_ERROR
 */
int	cmd_connect(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc);

/**
 * cmd_close
 *
 * Close the connection
 *
 * @arg	cli	the cli struct to update
 *
 * @return	CLI_OK or CLI_ERROR
 */
int	cmd_close(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc));

/**
 * cmd_hello
 *
 * Implements the hello RPC call
 *
 * @return	CLI_OK or CLI_ERROR
 */
int	cmd_hello(UNUSED(struct cli_def *cli), UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc));

/**
 * clear_node
 *
 * Clears a node
 *
 * @arg	the node to clear
 */
void	clear_node(rpc::t_node _return);

/**
 * set_prompt
 *
 * Updates the prompt (memory allocation)
 *
 * @arg	prompt	the target to update
 * @arg new_prompt	the new prompt
 *
 * @return	true on success
 */
bool	set_prompt(char* prompt, const char* new_prompt);


/**
 * main
 *
 * This is the main function of the program
 *
 * @return	EXIT_SUCCESSS or EXIT_FAILURE
 */
int	main(const int argc, char const* argv[]);

#endif // _MAIN_H_
