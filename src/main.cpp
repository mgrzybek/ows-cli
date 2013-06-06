/**
 * Project: ows-cli: a shell client for Open Workload Scheduler
 * File name: main.cpp
 * Description: implements the main file
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

#include "main.h"

///////////////////////////////////////////////////////////////////////////////

#define RPC_EXEC(command) \
try { \
	if ( client.get_handler() == NULL ) { \
		printf("Not connected!\n"); \
		return CLI_ERROR; \
	} \
	command;\
} catch (const rpc::ex_routing& e) { \
	std::cerr << "ex::routing: " << e.msg << std::endl; \
	return CLI_ERROR; \
} catch (const rpc::ex_node& e) { \
	std::cerr << "ex::node: " << e.msg << std::endl; \
	return CLI_ERROR; \
} catch (const rpc::ex_processing& e) { \
	std::cerr << "ex_processing: " << e.msg << std::endl; \
	return CLI_ERROR; \
} catch (...) { \
	std::cerr << "Undefined exception occured" << std::endl; \
	return CLI_ERROR; \
}

///////////////////////////////////////////////////////////////////////////////

bool	cli_add_commands(struct cli_def* cli) {
	struct cli_command*	c = NULL;

	// connection
	cli_register_command(cli, NULL, "connect", cmd_connect, PRIVILEGE_UNPRIVILEGED, MODE_DISCONNECTED, "Connect against a node");
	cli_register_command(cli, NULL, "close", cmd_close, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Disconnect");
	cli_register_command(cli, NULL, "hello", cmd_hello, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Sends a hello request");

	// add
	c = cli_register_command(cli, NULL, "add", NULL, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, NULL);
	cli_register_command(cli, c, "node", cmd_add_node, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, "Add a node");

	// remove
	c = cli_register_command(cli, NULL, "remove", NULL, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, NULL);
	cli_register_command(cli, c, "node", cmd_remove_node, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, "remove a node");

	// get
	c = cli_register_command(cli, NULL, "get", NULL, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, NULL);
	cli_register_command(cli, c, "nodes", cmd_get_nodes, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Show the available nodes");
	cli_register_command(cli, c, "jobs", cmd_get_jobs, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Show the available jobs");
	cli_register_command(cli, c, "ready jobs", cmd_get_ready_jobs, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Show the ready jobs");

	c = NULL;
	return true;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_get_nodes(UNUSED(struct cli_def *cli), UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc)) {
	rpc::v_nodes	nodes;

	RPC_EXEC(client.get_handler()->get_nodes(nodes, local_node.domain_name, local_node, target_node))

	std::for_each(nodes.begin(), nodes.end(), print_node);

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_add_node(UNUSED(struct cli_def *cli), UNUSED(const char *command), char *argv[], int argc) {
	rpc::t_node	node_to_add;
	std::string	line;
	std::string	key;
	std::string	value;
	boost::regex	comment("^#.*?$", boost::regex::perl);

	std::cout << "argc == " << argc << std::endl;

	if ( argc != 0 ) {
		// Parse the arguments
		for ( int i = 0 ; i < argc ; i++ ) {
			line = argv[i];

			if ( boost::regex_match(line, comment) == true || line.length() == 0 )
				continue;

			if ( split_line('=',line, key, value) == false ) {
				return CLI_ERROR_ARG;
			}

			update_node(key, value, node_to_add);
		}
	} else {
		// Parse std::cin
		while ( std::cin >> line) {
			if ( boost::regex_match(line, comment) == true || line.length() == 0 )
				continue;

			if ( split_line('=', line, key, value) == false )
				return CLI_ERROR_ARG;

			update_node(key, value, node_to_add);
		}

		std::cerr << "Reading the input is not implemented yet!" << std::endl;
		return CLI_ERROR_ARG;
	}

	RPC_EXEC(client.get_handler()->add_node(local_node.domain_name, local_node, target_node, node_to_add))

	return CLI_OK;
}

int	cmd_remove_node(UNUSED(struct cli_def *cli), UNUSED(const char *command), char *argv[], int argc) {
	rpc::t_node	node_to_remove;
//	boost::regex	spaces("[[:space:]]+", boost::regex::perl);
//	boost::regex	comment("^#.*?$", boost::regex::perl);
//	boost::regex	comment_endl("#.*?$", boost::regex::perl);

	if ( argc != 1 ) {
		std::cerr << "Missing one argument" << std::endl;
		return CLI_ERROR_ARG;
	}

	node_to_remove.name = argv[0];
	node_to_remove.domain_name = local_node.domain_name;

	RPC_EXEC(client.get_handler()->add_node(local_node.domain_name, local_node, target_node, node_to_remove))

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_get_ready_jobs(UNUSED(struct cli_def *cli), UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc)) {
	rpc::v_jobs	ready_jobs;

	RPC_EXEC(client.get_handler()->get_ready_jobs(ready_jobs, local_node.domain_name, local_node, target_node))

	print_jobs(ready_jobs);

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_get_jobs(UNUSED(struct cli_def *cli), UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc)) {
	rpc::v_jobs	jobs;

	RPC_EXEC(client.get_handler()->get_jobs(jobs, local_node.domain_name, local_node, target_node))

	BOOST_FOREACH(rpc::t_job job, jobs) {
		print_job(job);
	}

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_connect(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc) {
	rpc::t_hello	hello_result;
	int		port = 8080;

	if ( argc != 2 && argc != 3 ) {
		std::cerr << "2 or 3 args are required: <domain> <hostname> [port]" << std::endl;
		return CLI_ERROR_ARG;
	}

	target_node.domain_name = argv[0];
	local_node.domain_name = argv[0];
	local_node.name = "ows-cli";

	if ( argc == 3 )
		port = boost::lexical_cast<int>(argv[2]);

	// TODO: check the port using a regex "\d" to avoid exceptions from the lexical_cast
	if ( client.open(argv[1], boost::lexical_cast<int>(port)) == false )
		return CLI_ERROR;

	// Updating the node
	target_node.name = argv[1];

	RPC_EXEC(client.get_handler()->hello(hello_result, target_node))

	target_node.domain_name = hello_result.domain;
	target_node.name = hello_result.name;

	cli->mode = MODE_CONNECTED;

	free(cli->promptchar);
	cli->promptchar = (char*) calloc(sizeof(char), strlen(target_node.name.c_str()));
	if ( sprintf(cli->promptchar, "%s> ", target_node.name.c_str()) == 0 ) {
		printf("Cannot update the prompt!\n");
		return CLI_ERROR;
	}

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_close(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc)) {
	if ( client.close() == false )
		return CLI_ERROR;

	clear_node(target_node);

	cli->mode = MODE_EXEC;
	cli_set_configmode(cli, MODE_DISCONNECTED, 0);

	free(cli->promptchar);
	set_prompt(cli->promptchar, "> ");

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_hello(UNUSED(struct cli_def *cli), UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc)) {
	rpc::t_hello	hello_result;

	RPC_EXEC(client.get_handler()->hello(hello_result, target_node))

	std::cout << "domain: " << hello_result.domain << std::endl
	<< "master: " << hello_result.is_master << std::endl
	<< "name: " << hello_result.name << std::endl;

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

void	clear_node(rpc::t_node _return) {
	_return.name.clear();
	_return.domain_name.clear();
	_return.jobs.clear();
	_return.resources.clear();
	_return.weight = -1;
}

///////////////////////////////////////////////////////////////////////////////

bool	set_prompt(char* prompt, const char* new_prompt) {
	if ( new_prompt == NULL )
		return false;

	if ( prompt != NULL && !prompt )
		delete prompt;

	prompt = (char*) calloc(sizeof(char), strlen(new_prompt));
	if ( snprintf(prompt, strlen(new_prompt), "%s", new_prompt) < 0 )
		return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////

int	main(const int argc, char const* argv[]) {
	struct cli_def*	cli = NULL;
	bool	interactive = true;

	/*
	 * Args checking
	 * Do you open stdin or use a shell?
	 */
	if ( argc == 2 && strcmp(argv[1], "-") == 0 ) {
		interactive = false;
	}

	/*
	 * Init the commands
	 */
	cli = cli_init();
	if ( cli == 0 ) {
		std::cerr << "Cannot allocate memory!" << std::endl;
		return EXIT_FAILURE;
	}

	if ( cli_add_commands(cli) == false )
		return EXIT_FAILURE;

	/*
	 * Processing
	 */
	if ( interactive == true ) {
		/*
		 * Do we have the domain and hostname as arguments?
		 */
		if ( argc == 3 ) {
			std::string cmd = "connect ";
			cmd += argv[1];
			cmd += " ";
			cmd += argv[2];
			cli_run_command(cli, cmd.c_str());
		}

		if ( cli_loop(cli) == CLI_ERROR ) {
			std::cerr << "Cannot start the shell!" << std::endl;
			return EXIT_FAILURE;
		}
		cli_done(cli);
	} else {
		cli_file(cli, stdin, PRIVILEGE_UNPRIVILEGED, MODE_EXEC);
	}

	return EXIT_SUCCESS;
}
