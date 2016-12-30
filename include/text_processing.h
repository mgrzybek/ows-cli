/**
 * Project: ows-cli: a shell client for Open Workload Scheduler
 * File name: text_processing.h
 * Description: describes the function used to get data from text and update objects
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

#include <iostream>
#include <string>
#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include "convertions.h"
#include "model_types.h"


/**
 * @brief The e_output_type enum
 *
 * Used to print result in JSON or plain text.
 */
enum e_output_type {
	json,
	plain
};

struct s_printing_options {
	bool			verbose	= false;
	e_output_type	output_type = plain;
	char			indent_character = '	';
};

/**
 * @brief recovery_type_to_string
 * @param t
 * @return the type as string
 */
std::string	recovery_type_action_to_string(const rpc::t_recovery_type& t);

/**
 * @brief time_constraint_type_to_string
 * @param t
 * @return the enum type as a string
 */
std::string	time_constraint_type_to_string(const rpc::e_time_constraint_type::type& t);

/**
 * @brief time_constraint_to_string
 * @param tc
 * @return a comma-separated string
 */
std::string	time_constraint_to_string(const rpc::t_time_constraint& tc);

/**
 * @brief strings_to_string
 * @param v
 * @return a comma-separated string
 */
std::string	strings_to_string(const std::vector<std::string>& v);

/**
 * @brief bool_to_string
 * @param v
 * @return "true" or "false"
 */
std::string	bool_to_string(const bool& v);

/**
 * update_node
 *
 * Update the node's attribute according to the given key + value couple
 *
 * @param	key	the key to update
 * @param	value	the value to record
 * @param	node	the node to update
 */
void	update_node(const std::string& key, const std::string& value, rpc::t_node& node);

/**
 * update_job
 *
 * Update the job's attribute according to the given key + value couple
 *
 * time_constraint pattern: time_constraint=(AFTER|AT|BEFORE):hhmm((,|;)(AFTER|AT|BEFORE):hhmm)*
 * hh: hours (2 digits)
 * mm: minutes (2 digits)
 *
 * @param	key	the key to update
 * @param	value	the value to record
 * @param	job	the job to update
 */
void	update_job(const std::string& key, const std::string& value, rpc::t_job& job);

/**
 * split_line
 *
 * Split a 'key=value' line
 *
 * @param	separator	the token to use to split
 * @param	data	the line to split
 * @param	key	the returned key
 * @param	value	the returned value
 *
 * @return	true on success
 */
bool	split_line(const char& separator, const std::string& data, std::string& key, std::string& value);
