/**
 * Project: ows-cli: a shell client for Open Workload Scheduler
 * File name: text_processing.cpp
 * Description: implements the function used to get data from text and update objects
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

#include "text_processing.h"

void	update_node(const std::string& key, const std::string& value, rpc::t_node node) {
	if ( key.compare("name") == 0 ) {
		node.name = value;
	} else if ( key.compare("weight") == 0 ) {
		node.weight = boost::lexical_cast<int>(value);
	} else if ( key.compare("domain_name") == 0 ) {
		node.domain_name = value;
	} else if ( key.compare("resources") == 0 ) {
		// TODO
	} else if ( key.compare("jobs") == 0 ) {
		// TODO
	}
}

bool	split_line(const char& separator, const std::string& data, std::string& key, std::string& value) {
	boost::regex	spaces("[[:space:]]+", boost::regex::perl);
	boost::regex	comment_endl("#.*?$", boost::regex::perl);

	std::string	line = data;
	size_t		position = 0;

	line = boost::regex_replace(line, spaces, "");
	line = boost::regex_replace(line, comment_endl, "");


	position = line.find_first_of(separator);

	if ( position == std::string::npos ) {
		std::cerr << "No separator '" << separator << "' found" << std::endl;
		return false;
	}

	key	= line.substr(0, position);
	value	= line.substr(position+1, line.length());

	if ( key.length() == 0 or value.length() == 0 ) {
		std::cerr << "Bad input data (key or value empty)" << std::endl;
		return false;
	}

	return true;
}
