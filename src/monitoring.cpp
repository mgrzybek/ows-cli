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
	std::cout << "ex_routing: " << e.msg; \
	return false; \
} catch (const rpc::ex_node& e) { \
	std::cout << "ex_node: " << e.msg; \
	return false; \
} catch (const rpc::ex_processing& e) { \
	std::cout << "ex_processing: " << e.msg; \
	return false; \
} catch (...) { \
	std::cout << "Undefined exception occured"; \
	return false; \
}

void	usage(void) {
	std::cout << "monitoring -H <hostname> -p <port> -w <warning> -c <critical> -m <metric> -d <domain>" << std::endl;
	std::cout << "	<hostname>	: the node to check" << std::endl;
	std::cout << "	<port>		: the port to use" << std::endl;
	std::cout << "	<warning>	: the warning threshold" << std::endl;
	std::cout << "	<critical>	: the critical threshold" << std::endl;
	std::cout << "	<metric>	: the value to check (failed_jobs, waiting_jobs...)" << std::endl;
	std::cout << "	<domain>	: the domain's name to check" << std::endl;
}

bool	get_metric(rpc::integer& result, const std::string& metric, Rpc_Client& client, const rpc::t_routing_data& routing) {
	if ( metric.compare("failed_jobs") == 0 ) {
		RPC_EXEC_INTEGER_RETURN(client.get_handler()->monitor_failed_jobs(routing))
	} else if ( metric.compare("waiting_jobs") == 0 ) {
		RPC_EXEC_INTEGER_RETURN(client.get_handler()->monitor_waiting_jobs(routing))
	} else {
		return false;
	}

	return true;
}

// TODO: implement verbosity level (http://nagiosplug.sourceforge.net/developer-guidelines.html)
// TODO: think of using a GNU arg parsing lib instead of writing everything
int	main(const int argc, char const* argv[]) {
	/*
	 * Connection
	 */
	Rpc_Client	client;
	rpc::t_routing_data routing;
	int	port = -1;

	/*
	 * Monitoring
	 */
	rpc::integer	warning_threshold = -1;
	rpc::integer	critical_threshold = -1;
	rpc::integer	result = -1;
	std::string		metric = "";
	std::string		perfdata = "";
	std::string		message = "";

	std::vector<std::string>	metrics;
	metrics.push_back("failed_jobs");
	metrics.push_back("waiting_jobs");

	/*
	 * Output
	 */
	int	return_code = MON_UNKNOWN;

	/*
	 * Check the number of arguments
	 */
	if ( argc != 13 ) {
		usage();
		return MON_UNKNOWN;
	}

	/*
	 * Getting the args
	 */
	for (int i = 1 ; i < argc ; i += 2) {
		if (strcmp(argv[i], "-H") == 0) {
			routing.target_node.name = argv[i+1];
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
		if (strcmp(argv[i], "-d") == 0) {
			routing.target_node.domain_name = argv[i+1];
			routing.calling_node.domain_name = argv[i+1];
		}
	}

	/*
	 * Checking for missing args
	 */
	if ( routing.target_node.name.size() == 0 || port == -1 || warning_threshold == -1 || critical_threshold == -1 || metric.size() == 0 || routing.target_node.domain_name.size() == 0 ) {
		std::cout << "Missing args!" << std::endl;
		usage();
		return MON_UNKNOWN;
	}

	/*
	 * RPC call
	 */
	routing.calling_node.name = "monitoring";

	client.open(routing.target_node.name.c_str(), port);

	if ( metric.compare("all") == 0 ) {
		BOOST_FOREACH(std::string command, metrics) {
			if ( get_metric(result, command, client, routing) == false ) {
				client.close();
				return MON_UNKNOWN;
			}

			perfdata += command;
			perfdata += "=";
			perfdata += boost::lexical_cast<std::string>(result);
			perfdata += ";";
		}
	} else {
		if ( get_metric(result, metric, client, routing) == false ) {
			client.close();
			return MON_UNKNOWN;
		}

		perfdata += metric;
		perfdata += "=";
		perfdata += boost::lexical_cast<std::string>(result);
		perfdata += ";";
	}

	client.close();

	/*
	 * Printing the result
	 * TODO: not sure of the output format, need to check
	 */
	std::cout << metric;

	if ( result < warning_threshold ) {
		std::cout << " is fine";
		return_code = MON_OK;
	} else if ( result >= warning_threshold && result < critical_threshold ) {
		std::cout << " is high";
		return_code = MON_WARNING;
	} else {
		std::cout << " is too high";
		return_code = MON_CRITICAL;
	}

	// Let's remove the last ';'
	perfdata.resize(perfdata.size() - 1);

	std::cout << "|" << perfdata;

	return return_code;
}
