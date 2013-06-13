/**
 * Project: ows-cli: a shell client for Open Workload Scheduler
 * File name: monitoring.h
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

#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "rpc_client.h"

#define MON_OK	0
#define MON_WARNING	1
#define MON_CRITICAL	2
#define MON_UNKNOWN	3

/**
 * usage
 *
 * Prints the usage message
 */
void	usage(void);

/**
 * get_metric
 *
 * @param result	: the result to get
 * @param metric	: the name of the metric
 * @param client	: the connection object
 * @param local_node	: the caller's identity
 * @param target_node	: the target
 *
 * @return	true on success
 */
bool	get_metric(rpc::integer& result, const std::string& metric, Rpc_Client& client, const rpc::t_node& local_node, const rpc::t_node& target_node);

/**
 * main
 *
 * @param	argc : the number of arguments
 * @param	argv : the arguments
 *
 * @return	MON_OK or MON_WARNING or MON_CRITICAL or MON_UNKNOWN
 */
int	main(const int argc, char const* argv[]);

#endif // MAIN_H
