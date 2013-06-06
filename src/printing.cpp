/**
 * Project: ows-cli: a shell client for Open Workload Scheduler
 * File name: printing.cpp
 * Description: implements the function used to print data on the screen
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

#include "printing.h"

void	print_node(const rpc::t_node& node) {
	std::cout << "domain:	" << node.domain_name << std::endl;
	std::cout << "name:	" << node.name << std::endl;
	std::cout << "weight:	" << node.weight << std::endl;

	std::cout << "jobs:	" << std::endl;
	BOOST_FOREACH(rpc::t_job job, node.jobs) {
		print_job(job);
	}

	std::cout << "resources:" << std::endl;
	BOOST_FOREACH(rpc::t_resource resource, node.resources) {
		std::cout << "	resource..." << std::endl;
	}
}

void	print_jobs(const rpc::v_jobs& jobs) {
	std::for_each(jobs.begin(), jobs.end(), print_job);
}

void	print_job(const rpc::t_job& job) {
	std::cout << "	name:	" << job.name;
	std::cout << "	state:	" << build_string_from_job_state(job.state) << std::endl;
}
