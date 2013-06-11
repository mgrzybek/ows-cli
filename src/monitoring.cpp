/**
 * Project: ows-cli: a shell client for Open Workload Scheduler
 * File name: monitoring.cpp
 * Description: describes the monitoring binary
 *
 * @author Mathieu Grzybek on 2013-06-11
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

#include "monitoring.h"

#define RPC_EXEC_INTEGER_RETURN(command) \
try { \
	if ( client.get_handler() == NULL ) { \
		std::cout << "Not connected!\n"; \
		return MON_UNKNOWN; \
	} \
	result = command;\
} catch (const rpc::ex_routing& e) { \
	std::cout << "ex::routing: " << e.msg; \
	return MON_UNKNOWN; \
} catch (const rpc::ex_node& e) { \
	std::cout << "ex::node: " << e.msg; \
	return MON_UNKNOWN; \
} catch (const rpc::ex_processing& e) { \
	std::cout << "ex_processing: " << e.msg; \
	return MON_UNKNOWN; \
} catch (...) { \
	std::cout << "Undefined exception occured"; \
	return MON_UNKNOWN; \
}

void	usage(void) {
	std::cout << "monitoring -H <hostname> -p <port> -w <warning> -c <critical> -m <metric>" << std::endl;
	std::cout << "	<hostname>	: the node to check" << std::endl;
	std::cout << "	<port>	: the port to use" << std::endl;
	std::cout << "	<warning>	: the warning threshold" << std::endl;
	std::cout << "	<critical>	: the critical threshold" << std::endl;
	std::cout << "	<metric>	: the value to check (failed_jobs, waiting_jobs...)" << std::endl;
}

// TODO: implement verbosity level (http://nagiosplug.sourceforge.net/developer-guidelines.html)
// TODO: think of using a GNU arg parsing lib instead of writing everything
int	main(const int argc, char const* argv[]) {
	/*
	 * Connection
	 */
	Rpc_Client	client;
	rpc::t_node	local_node;
	rpc::t_node	target_node;
	int	port = -1;

	/*
	 * Monitoring
	 */
	rpc::integer	warning_threshold = -1;
	rpc::integer	critical_threshold = -1;
	rpc::integer	result = -1;
	std::string	metric = "";

	/*
	 * Output
	 */
	int	return_code = MON_UNKNOWN;

	/*
	 * Check the number of arguments
	 */
	if ( argc != 11 ) {
		usage();
		return MON_UNKNOWN;
	}

	/*
	 * Getting the args
	 */
	for (int i = 1 ; i < argc ; i += 2) {
		if (strcmp(argv[i], "-H") == 0) {
			target_node.name = argv[i+1];
		}
		if (strcmp(argv[i], "-p") == 0) {
			port = boost::lexical_cast<int>(argv[i+1]);
		}
		if (strcmp(argv[i], "-w") == 0) {
			warning_threshold = boost::lexical_cast<rpc::integer>(argv[i+1]);
		}
		if (strcmp(argv[i], "-c") == 0) {
			critical_threshold = boost::lexical_cast<rpc::integer>(argv[i+1]);
		}
		if (strcmp(argv[i], "-m") == 0) {
			metric = argv[i+1];
		}
	}

	/*
	 * Checking for missing args
	 */
	if ( target_node.name.size() == 0 || port == -1 || warning_threshold == -1 || critical_threshold == -1 || metric.size() == 0 ) {
		std::cout << "Missing args!" << std::endl;
		usage();
		return MON_UNKNOWN;
	}

	/*
	 * RPC call
	 */
	local_node.name = "monitoring";
	local_node.domain_name = target_node.domain_name;

	client.open(target_node.name.c_str(), port);

	if ( metric.compare("failed_jobs") == 0 ) {
		RPC_EXEC_INTEGER_RETURN(client.get_handler()->monitor_failed_jobs(local_node.domain_name, local_node, target_node))
	} else if ( metric.compare("waiting_jobs") == 0 ) {
		RPC_EXEC_INTEGER_RETURN(client.get_handler()->monitor_waiting_jobs(local_node.domain_name, local_node, target_node))
	} else {
		std::cout << "Cannot find the metric";
		client.close();
		return MON_UNKNOWN;
	}

	client.close();

	/*
	 * Printing the result
	 * TODO: not sure of the output format, need to check
	 */
	if ( result < warning_threshold ) {
		std::cout << "OK|" << metric << " is fine";
		return_code = MON_OK;
	} else if ( result >= warning_threshold && result < critical_threshold ) {
		std::cout << "WARNING|" << metric << " is high";
		return_code = MON_WARNING;
	} else {
		std::cout << "CRITICAL|" << metric << " is too high";
		return_code = MON_CRITICAL;
	}

	std::cout << "|" << metric << "=" << result;

	return return_code;
}
