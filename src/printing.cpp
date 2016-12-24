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

// TODO: http://stackoverflow.com/questions/8806643/colorized-output-breaks-linewrapping-with-readline

void	get_indent(const s_printing_options& opts, const uint& number, std::string& _return) {
	static uint _number = 0;

	if ( _number > number ) {
		for ( uint i = _return.size() ; i < number ; ++i ) {
			_return += opts.indent_character;
		}
	} else {
		if ( _number < number ) {
			_return = "";
			for ( uint i = 0 ; i < number ; ++i ) {
				_return += opts.indent_character;
			}
		}
	}
}

void	print_kv(const s_printing_options& opts, const uint& indent, const m_kv& kv) {
	if ( opts.output_type == plain )
		for ( const auto& pair : kv ) {
			if ( pair.first.compare("command") == 0 && opts.verbose == false )
				continue;
			std::cout << pair.first << ": " << pair.second << std::endl;
		}
	else if ( opts.output_type == json ) {
		size_t counter = 1;

		std::cout << "{";
		if ( opts.verbose == true ) {
			std::cout << std::endl << "'command':'" << kv.at("command") << "'," << std::endl;
			std::cout << "'result':{";
		}
		for ( const auto& pair : kv ) {
			++counter;

			if ( pair.first.compare("command") == 0 )
				continue;

			std::cout << "'" << pair.first << "':'" << pair.second << "'";

			if ( counter < kv.bucket_count() - 1)
				std::cout << ",";
		}
		if ( opts.verbose == true ) {
			std::cout << "}" << std::endl;
		}
		std::cout << "}" << std::endl;
	}
}

void	print_nodes(const s_printing_options& opts, const uint& indent, const rpc::v_nodes& nodes) {
	if ( opts.output_type == plain ) {
		BOOST_FOREACH(rpc::t_node node, nodes) {
			print_node(opts, indent + 1, node);
		}
	} else {
		std::cout << "[";
		BOOST_FOREACH(rpc::t_node node, nodes) {
			std::cout << std::endl;
			print_node(opts, indent + 1, node);
		}
		std::cout << "]" << std::endl;
	}
}

void	print_node(const s_printing_options& opts, const uint& indent, const rpc::t_node& node) {
	std::string	str_indent;
	get_indent(opts, indent, str_indent);

	if ( opts.output_type == plain ) {
		std::cout << str_indent << "domain:	" << node.domain_name << std::endl;
		std::cout << str_indent << "name:	" << node.name << std::endl;
		std::cout << str_indent << "weight:	" << node.weight << std::endl;

		std::cout << str_indent << "jobs:	" << std::endl;
		print_jobs(opts, indent + 2, node.jobs);

		std::cout << str_indent << "resources:" << std::endl;
		print_resources(opts, indent + 2, node.resources);
	} else {
		std::cout << str_indent << "{" << std::endl;

		get_indent(opts, indent + 1, str_indent);
		std::cout << str_indent << "'domain':'" << node.domain_name << "'," << std::endl
				  << str_indent << "'name':'" << node.name << "," << std::endl
				  << str_indent << "'weight':" << node.weight << "," << std::endl;

		std::cout << str_indent << "'jobs':";
		print_jobs(opts, indent + 2, node.jobs);
		std::cout << "," << std::endl
				  << str_indent << "'resources':";

		print_resources(opts, indent + 2, node.resources);

		get_indent(opts, indent, str_indent);
		std::cout << std::endl << str_indent << "}" << std::endl;
	}
}

void	print_jobs(const s_printing_options& opts, const uint& indent, const rpc::v_jobs& jobs) {
	std::string	str_indent;

	if ( jobs.size() == 0 ) {
		if ( opts.output_type == json )
			std::cout << "[]";
		return;
	}

	get_indent(opts, indent, str_indent);

	if ( opts.output_type == plain ) {
		BOOST_FOREACH(rpc::t_job job, jobs) {
			print_job(opts, indent, job);
		}
	} else {
		std::cout << "[" << std::endl;
		BOOST_FOREACH(rpc::t_job job, jobs) {
			print_job(opts, indent, job);
			std::cout << std::endl;
		}
		get_indent(opts, indent, str_indent);
		std::cout << str_indent << "]";
	}
}

void	print_job(const s_printing_options& opts, const uint& indent, const rpc::t_job& job) {
	std::string	str_indent;
	get_indent(opts, indent, str_indent);

	if ( opts.output_type == plain ) {
		std::cout << str_indent << "name:	" << job.name << std::endl;
		std::cout << str_indent << "state:	" << build_string_from_job_state(job.state) << std::endl;
	} else {
		std::cout << str_indent << "{'name':'" << job.name << "','state':'" << build_string_from_job_state(job.state) << "'}";
	}
}

void	print_resources(const s_printing_options& opts, const uint& indent, const rpc::v_resources& resources) {
	std::string	str_indent;

	if ( resources.size() == 0 ) {
		if ( opts.output_type == json )
			std::cout << "[]";
		return;
	}

	get_indent(opts, indent, str_indent);

	if ( opts.output_type == plain ) {
		BOOST_FOREACH(rpc::t_resource resource, resources) {
			print_resource(opts, indent, resource);
		}
	} else {
		std::cout << "[" << std::endl;
		BOOST_FOREACH(rpc::t_resource resource, resources) {
			print_resource(opts, indent, resource);
			std::cout << std::endl;
		}
		get_indent(opts, indent, str_indent);
		std::cout << str_indent << "]";
	}
}

void	print_resource(const s_printing_options& opts, const uint& indent, const rpc::t_resource& resource) {
	std::string	str_indent;
	get_indent(opts, indent, str_indent);

	if ( opts.output_type == plain ) {
		std::cout << str_indent << "name:	" << resource.name << std::endl;
		std::cout << str_indent << "current value:	" << resource.current_value << std::endl;
		std::cout << str_indent << "initial value:	" << resource.initial_value << std::endl;
	} else {
		std::cout << str_indent << "{'name':'" << resource.name << "','current value':" << resource.current_value << "','initial value':" << resource.initial_value << "}";
	}
}
