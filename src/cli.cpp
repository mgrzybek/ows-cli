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

#include "cli.h"

///////////////////////////////////////////////////////////////////////////////

#define VERBOSE_PRINT(text) \
if ( print_opts.verbose ) \
	std::cout << text << std::endl;

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
} catch (const rpc::ex_job& e) { \
	std::cerr << "ex::job: " << e.msg << std::endl; \
	return CLI_ERROR; \
} catch (const rpc::ex_processing& e) { \
	std::cerr << "ex_processing: " << e.msg << std::endl; \
	return CLI_ERROR; \
} catch (std::exception& e) { \
	std::cerr << "Undefined exception occured:" << e.what() << std::endl; \
	return CLI_ERROR; \
}

///////////////////////////////////////////////////////////////////////////////

#define RPC_EXEC_RESULT_RETURN(command) \
try { \
	if ( client.get_handler() == NULL ) { \
		printf("Not connected!\n"); \
		return CLI_ERROR; \
	} \
	result = command;\
} catch (const rpc::ex_routing& e) { \
	std::cerr << "ex::routing: " << e.msg << std::endl; \
	return CLI_ERROR; \
} catch (const rpc::ex_node& e) { \
	std::cerr << "ex::node: " << e.msg << std::endl; \
	return CLI_ERROR; \
} catch (const rpc::ex_job& e) { \
	std::cerr << "ex::job: " << e.msg << std::endl; \
	return CLI_ERROR; \
} catch (const rpc::ex_processing& e) { \
	std::cerr << "ex_processing: " << e.msg << std::endl; \
	return CLI_ERROR; \
} catch (const std::exception& e) { \
	std::cerr << "Undefined exception occured:" << e.what() << std::endl; \
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
	cli_register_command(cli, c, "job", cmd_add_job, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, "Add a job");
	cli_register_command(cli, c, "node", cmd_add_node, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, "Add a node");

	// remove
	c = cli_register_command(cli, NULL, "remove", NULL, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, NULL);
	cli_register_command(cli, c, "job", cmd_remove_job, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, "Remove a job");
	cli_register_command(cli, c, "node", cmd_remove_node, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, "Remove a node");

	// get
	c = cli_register_command(cli, NULL, "get", NULL, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, NULL);
	cli_register_command(cli, c, "nodes", cmd_get_nodes, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Show the available nodes");
	cli_register_command(cli, c, "jobs", cmd_get_jobs, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Show the available jobs");
	cli_register_command(cli, c, "ready jobs", cmd_get_ready_jobs, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Show the ready jobs");
	cli_register_command(cli, c, "current planning", cmd_get_current_planning_name, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Show the current planning name");
	cli_register_command(cli, c, "available plannings", cmd_get_available_planning_names, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Show the available planning names");

	// update
	c = cli_register_command(cli, NULL, "update", NULL, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, NULL);
	cli_register_command(cli, c, "job", cmd_update_job, PRIVILEGE_PRIVILEGED, MODE_CONNECTED, "Update a job");

	// use
	cli_register_command(cli, NULL, "use", cmd_use, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Use a planning");

	// monitor
	c = cli_register_command(cli, NULL, "monitor", NULL, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, NULL);
	cli_register_command(cli, c, "failed", cmd_monitor_failed_jobs, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Show the number of failed jobs");
	cli_register_command(cli, c, "waiting", cmd_monitor_waiting_jobs, PRIVILEGE_UNPRIVILEGED, MODE_CONNECTED, "Show the number of waiting jobs");

	c = NULL;
	return true;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_get_nodes(UNUSED(struct cli_def *cli), const char *command, UNUSED(char *argv[]), UNUSED(int argc)) {
	rpc::v_nodes	nodes;

	VERBOSE_PRINT(command)

	RPC_EXEC(client.get_handler()->get_nodes(nodes, routing))

	print_nodes(print_opts, 0, nodes);

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_add_node(UNUSED(struct cli_def *cli), const char *command, char *argv[], int argc) {
	rpc::t_node	node_to_add;
	std::string	line;
	std::string	key;
	std::string	value;
	bool		result;
	boost::regex	comment("^#.*?$", boost::regex::perl);

	VERBOSE_PRINT(command)

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
	}

	RPC_EXEC_RESULT_RETURN(client.get_handler()->add_node(routing, node_to_add))

	if ( result == true ) {
		std::cout << "success" << std::endl;
	} else {
		std::cout << "failure" << std::endl;
	}

	return CLI_OK;
}

int	cmd_remove_node(UNUSED(struct cli_def *cli), const char *command, char *argv[], int argc) {
	bool		result;
	rpc::t_node	node_to_remove;
	std::string key;
//	boost::regex	spaces("[[:space:]]+", boost::regex::perl);
//	boost::regex	comment("^#.*?$", boost::regex::perl);
//	boost::regex	comment_endl("#.*?$", boost::regex::perl);

	VERBOSE_PRINT(command)

	// TODO: check the number of arguments required by the CLI and the node
	if ( argc != 2 ) {
		std::cerr << "Missing one argument" << std::endl;
		return CLI_ERROR_ARG;
	}

	if ( split_line('=', argv[0], key, node_to_remove.name) == false )
		return CLI_ERROR_ARG;

	node_to_remove.domain_name = routing.calling_node.domain_name;

	RPC_EXEC_RESULT_RETURN(client.get_handler()->remove_node(routing, node_to_remove))

	if ( result == true ) {
		std::cout << "success" << std::endl;
	} else {
		std::cout << "failure" << std::endl;
	}

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_add_job(UNUSED(struct cli_def *cli), const char *command, char *argv[], int argc) {
	rpc::t_job	job_to_add;
	std::string	line;
	std::string	key;
	std::string	value;
	bool		result;
	boost::regex	comment("^#.*?$", boost::regex::perl);

	VERBOSE_PRINT(command)

	// By default the job belongs to the connected domain
	job_to_add.domain = routing.target_node.domain_name;

	if ( argc != 0 ) {
		// Parse the arguments
		for ( int i = 0 ; i < argc ; i++ ) {
			line = argv[i];

			if ( boost::regex_match(line, comment) == true || line.length() == 0 )
				continue;

			if ( split_line('=',line, key, value) == false ) {
				return CLI_ERROR_ARG;
			}

			update_job(key, value, job_to_add);
		}
	} else {
		// Parse std::cin
		while ( std::cin >> line) {
			if ( boost::regex_match(line, comment) == true || line.length() == 0 )
				continue;

			if ( split_line('=', line, key, value) == false )
				return CLI_ERROR_ARG;

			update_job(key, value, job_to_add);
		}

		std::cerr << "Reading the input is not implemented yet!" << std::endl;
		return CLI_ERROR_ARG;
	}

	// TODO: change add_job -> add target_node argument
	RPC_EXEC_RESULT_RETURN(client.get_handler()->add_job(routing, job_to_add))

	if ( result == true ) {
		std::cout << "success" << std::endl;
	} else {
		std::cout << "failure" << std::endl;
	}

	return CLI_OK;
}

int	cmd_remove_job(UNUSED(struct cli_def *cli), const char *command, char *argv[], int argc) {
	rpc::t_job	job_to_remove;
	std::string	key;
	std::string	value;
	std::string	line;
//	boost::regex	spaces("[[:space:]]+", boost::regex::perl);
	boost::regex	comment("^#.*?$", boost::regex::perl);
//	boost::regex	comment_endl("#.*?$", boost::regex::perl);
	bool	result;

	VERBOSE_PRINT(command)

	if ( argc != 2 ) {
		// Parse the arguments
		for ( int i = 0 ; i < argc ; i++ ) {
			line = argv[i];

			if ( split_line('=',line, key, value) == false ) {
				return CLI_ERROR_ARG;
			}

			update_job(key, value, job_to_remove);
		}
	} else {
		// Parse std::cin
		while ( std::cin >> line) {
			if ( boost::regex_match(line, comment) == true || line.length() == 0 )
				continue;

			if ( split_line('=', line, key, value) == false )
				return CLI_ERROR_ARG;

			update_job(key, value, job_to_remove);
		}

		std::cerr << "Reading the input is not implemented yet!" << std::endl;
		return CLI_ERROR_ARG;
	}

	job_to_remove.node_name = routing.target_node.name;
	job_to_remove.domain = routing.target_node.domain_name;

	RPC_EXEC_RESULT_RETURN(client.get_handler()->remove_job(routing, job_to_remove))

	if ( result == true)
		std::cout << "success";
	else
		std::cout << "failure";

	std::cout << std::endl;

	return CLI_OK;
}

int	cmd_update_job(UNUSED(struct cli_def *cli), const char *command, char *argv[], int argc) {
	rpc::t_job	job_to_update;
	std::string	line;
	std::string	key;
	std::string	value;
	boost::regex	comment("^#.*?$", boost::regex::perl);

	VERBOSE_PRINT(command)

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

			update_job(key, value, job_to_update);
		}
	} else {
		// Parse std::cin
		while ( std::cin >> line) {
			if ( boost::regex_match(line, comment) == true || line.length() == 0 )
				continue;

			if ( split_line('=', line, key, value) == false )
				return CLI_ERROR_ARG;

			update_job(key, value, job_to_update);
		}

		std::cerr << "Reading the input is not implemented yet!" << std::endl;
		return CLI_ERROR_ARG;
	}

	// TODO: change add_job -> add target_node argument
	RPC_EXEC(client.get_handler()->update_job(routing, job_to_update))

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_update_job_state(UNUSED(struct cli_def *cli), const char *command, char *argv[], int argc) {
	rpc::t_job	job;

	VERBOSE_PRINT(command)

	if ( argc != 2 ) {
		std::cerr << "Needs two arguments: job_name and job_state" << std::endl;
		return CLI_ERROR_ARG;
	}

	job.name = argv[0];
	job.state = build_job_state_from_string(argv[1]);

	RPC_EXEC(client.get_handler()->update_job_state(routing, job))

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_get_ready_jobs(UNUSED(struct cli_def *cli), const char *command, UNUSED(char *argv[]), UNUSED(int argc)) {
	rpc::v_jobs	ready_jobs;

	VERBOSE_PRINT(command)

	RPC_EXEC(client.get_handler()->get_ready_jobs(ready_jobs, routing))

	print_jobs(print_opts, 0, ready_jobs);
	std::cout << std::endl;

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_get_jobs(UNUSED(struct cli_def *cli), const char *command, UNUSED(char *argv[]), UNUSED(int argc)) {
	rpc::v_jobs	jobs;

	VERBOSE_PRINT(command)

	RPC_EXEC(client.get_handler()->get_jobs(jobs, routing))

	print_jobs(print_opts, 0, jobs);
	std::cout << std::endl;

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_monitor_failed_jobs(UNUSED(struct cli_def *cli), const char *command, UNUSED(char *argv[]), UNUSED(int argc)) {
	rpc::integer	result = 0;

	VERBOSE_PRINT(command)

	RPC_EXEC_RESULT_RETURN(client.get_handler()->monitor_failed_jobs(routing))

	std::cout << result << std::endl;

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_get_current_planning_name(UNUSED(struct cli_def *cli), const char *command, UNUSED(char *argv[]), UNUSED(int argc)) {
	std::string	result;

	VERBOSE_PRINT(command)

	RPC_EXEC(client.get_handler()->get_current_planning_name(result, routing))

	std::cout << result << std::endl;

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_get_available_planning_names(UNUSED(struct cli_def *cli), const char *command, UNUSED(char *argv[]), UNUSED(int argc)) {
	std::vector<std::string> result;

	VERBOSE_PRINT(command)

	RPC_EXEC(client.get_handler()->get_available_planning_names(result, routing))

	BOOST_FOREACH(std::string name, result) {
		std::cout << name << std::endl;
	}

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_monitor_waiting_jobs(UNUSED(struct cli_def *cli), const char *command, UNUSED(char *argv[]), UNUSED(int argc)) {
	rpc::integer	result = 0;

	VERBOSE_PRINT(command)

	RPC_EXEC_RESULT_RETURN(client.get_handler()->monitor_waiting_jobs(routing))

	std::cout << result << std::endl;

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_connect(struct cli_def *cli, const char *command, char *argv[], int argc) {
	rpc::t_hello	hello_result;
	int		port = 8080;
	boost::regex expr{"\\d+"};

	VERBOSE_PRINT(command)

	if ( argc != 2 && argc != 3 ) {
		std::cerr << "2 or 3 args are required: <domain> <hostname> [port]" << std::endl;
		return CLI_ERROR_ARG;
	}

	routing.target_node.domain_name = argv[0];
	routing.calling_node.domain_name = argv[0];
	routing.calling_node.name = "ows-cli";

	if ( argc == 3 ) {
		if ( boost::regex_match(argv[2], expr) == false ) {
			printf("The given port is not a number!\n");
			return CLI_ERROR;
		}
		port = boost::lexical_cast<int>(argv[2]);
	}

	// TODO: check the port using a regex "\d" to avoid exceptions from the lexical_cast
	if ( client.open(argv[1], boost::lexical_cast<int>(port)) == false )
		return CLI_ERROR;

	// Updating the node
	routing.target_node.name = argv[1];

	RPC_EXEC(client.get_handler()->hello(hello_result, routing.target_node))

	routing.target_node.domain_name = hello_result.domain;
	routing.target_node.name = hello_result.name;

	cli->mode = MODE_CONNECTED;

	free(cli->promptchar);
	cli->promptchar = (char*) calloc(sizeof(char), strlen(routing.target_node.name.c_str()) + strlen(routing.target_node.domain_name.c_str()) +3);
	if ( sprintf(cli->promptchar, "%s:%s> ", routing.target_node.name.c_str(), routing.target_node.domain_name.c_str()) == 0 ) {
		printf("Cannot update the prompt!\n");
		return CLI_ERROR;
	}

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_use(struct cli_def *cli, const char *command, char *argv[], int argc) {
	size_t	space_needed;

	VERBOSE_PRINT(command)

	if ( argc != 1 ) {
		std::cerr << "1 argument is required: <planning_name>" << std::endl;
		return CLI_ERROR_ARG;
	}

	if ( std::string("current").compare(argv[0]) == 0 ) {
		std::string	result = "";

		// We could create a dedicated RPC function such as "bool planning_exists(planning, routing)"
		RPC_EXEC(client.get_handler()->get_current_planning_name(result, routing))

		if ( result.size() > 0 ) {
			routing.target_node.domain_name = result;
			routing.calling_node.domain_name = result;
		}
	} else {
		std::vector<std::string> result;

		// We could create a dedicated RPC function such as "bool planning_exists(planning, routing)"
		RPC_EXEC(client.get_handler()->get_available_planning_names(result, routing))

		if ( result.size() == 0 ) {
			std::cerr << "No planning available" << std::endl;
			return CLI_ERROR;
		}

		if ( std::find(result.begin(), result.end(), argv[0]) != result.end() ) {
			routing.target_node.domain_name = argv[0];
			routing.calling_node.domain_name = argv[0];
		} else {
			std::cerr << "Cannot find the planning " << argv[0] << std::endl;
			return CLI_ERROR;
		}
	}

	free(cli->promptchar);
	space_needed = snprintf(NULL, 0, "%s:%s> ", routing.target_node.name.c_str(), routing.target_node.domain_name.c_str());
	cli->promptchar = (char*) malloc(space_needed);
	if ( snprintf(cli->promptchar, space_needed, "%s:%s> ", routing.target_node.name.c_str(), routing.target_node.domain_name.c_str()) == 0 ) {
		printf("Cannot update the prompt!\n");
		return CLI_ERROR;
	}
	return CLI_OK;

	std::cerr << "Cannot find the given planning" << std::endl;
	return CLI_ERROR;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_close(struct cli_def *cli, const char *command, UNUSED(char *argv[]), UNUSED(int argc)) {
	m_kv values = {
		{"command", std::string(command)}
	};

	print_kv(print_opts, 0, values);

	if ( client.close() == false )
		return CLI_ERROR;

	clear_node(routing.target_node);

	cli->mode = MODE_EXEC;
	cli_set_configmode(cli, MODE_DISCONNECTED, 0);

	free(cli->promptchar);
	set_prompt(cli->promptchar, "> ");

	return CLI_OK;
}

///////////////////////////////////////////////////////////////////////////////

int	cmd_hello(UNUSED(struct cli_def *cli), const char *command, UNUSED(char *argv[]), UNUSED(int argc)) {
	rpc::t_hello	hello_result;

	RPC_EXEC(client.get_handler()->hello(hello_result, routing.target_node))

	m_kv values = {
		{"command",	std::string(command)},
		{"domain",	std::string(hello_result.domain)},
		{"master",	bool_to_string(hello_result.is_master)},
		{"name",	std::string(hello_result.name)}
	};

	print_kv(print_opts, 0, values);

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

void	usage() {
	std::cout << "ows-cli [(<domain_name> <hostname>) | -]" << std::endl;
	std::cout << "	<domain_name>	: the domain's name to connect against" << std::endl;
	std::cout << "	<hostname>	: the node to connect against" << std::endl;
	std::cout << "	-		: read stdin as input" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////

int	main(const int argc, char const* argv[]) {
	struct cli_def*	cli = NULL;
	bool	interactive = true;
	boost::program_options::variables_map opts_variables;

	/*
	 * Args checking
	 * Do we open stdin or use a shell?
	 */
	if ( argc == 2 && strcmp(argv[1], "-") == 0 ) {
		interactive = false;
	} else {

		boost::program_options::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce help")
			("verbose,v", "set verbosity on")
			("non-interactive,", "read stdin as input")
			("output", boost::program_options::value<std::string>(), "the output format (plain or json)")
			("domain", boost::program_options::value<std::string>(), "the domain to use")
			("hostname", boost::program_options::value<std::string>(), "the endpoint")
			("planning", boost::program_options::value<std::string>(), "the planning to use")
		;

		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), opts_variables);
		boost::program_options::notify(opts_variables);

		if (opts_variables.count("help")) {
			std::cout << desc << "\n";
			return EXIT_SUCCESS;
		}

		if ( opts_variables.count("non-interactive")) {
			VERBOSE_PRINT("non-interactive mode set to true")
			interactive = false;
		}

		if ( opts_variables.count("verbose")) {
			VERBOSE_PRINT("verbose set to true")
			print_opts.verbose = true;
		}

		if ( opts_variables.count("output")) {
			std::string _output = opts_variables["output"].as<std::string>();
			if ( _output.compare("plain") == 0 ) {
				VERBOSE_PRINT("output set to plain")
				print_opts.output_type = plain;
			} else {
				if ( _output.compare("json") == 0 ) {
					VERBOSE_PRINT("output set to json")
					print_opts.output_type = json;
				} else {
					std::cerr << "bad output format" << std::endl;
					return EXIT_FAILURE;
				}
			}
		}
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
		if ( opts_variables.count("domain") && opts_variables.count("hostname") ) {
			std::string cmd = "connect ";
			cmd += opts_variables["domain"].as<std::string>();
			cmd += " ";
			cmd += opts_variables["hostname"].as<std::string>();
			cli_run_command(cli, cmd.c_str());
		}

		/*
		 * Do we have a given planning as argument?
		 */
		if ( opts_variables.count("planning") ) {
			std::string cmd = "use ";
			cmd += opts_variables["planning"].as<std::string>();
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
