/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: convertions.cpp
 * Description: defines functions to use in the project:
 * - convert rpc:: types to std ones
 *
 * @author Mathieu Grzybek on 2016-12-31
 * @copyright 2016 Mathieu Grzybek. All rights reserved.
 * @version $Id: code-gpl-license.txt,v 1.2 2004/05/04 13:19:30 garry Exp $
 *
 * @see The GNU Public License (GPL) version 3 or higher
 *
 *
 * OWS is free software; you can redistribute it and/or modify
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
#ifndef CONVERTIONS_H
#define CONVERTIONS_H

#include <boost/regex.hpp>

#include "rpc_client.h"

/**
 * @brief build_job_state_from_string
 * @param state
 * @return
 */
rpc::e_job_state::type	build_job_state_from_string(const char* state);

/**
 * @brief build_string_from_job_state
 * @param js
 * @return
 */
std::string	build_string_from_job_state(const rpc::e_job_state::type& js);

/**
 * @brief build_rectype_action_from_string
 * @param rt_action
 * @return
 */
rpc::e_rectype_action::type	build_rectype_action_from_string(const char* rt_action);

/**
 * @brief build_string_from_rectype_action
 * @param rt_action
 * @return
 */
std::string	build_string_from_rectype_action(const rpc::e_rectype_action::type& rt_action);

/**
 * @brief build_time_constraint_type_from_string
 * @param type
 * @return
 */
rpc::e_time_constraint_type::type build_time_constraint_type_from_string(const char* type);

/**
 * @brief build_string_from_time_constraint_type
 * @param tc_t
 * @return
 */
std::string	build_string_from_time_constraint_type(const rpc::e_time_constraint_type::type& tc_t);

/**
 * @brief build_human_readable_time
 * @param time
 * @return
 */
std::string	build_human_readable_time(const time_t& time);

/**
 * @brief build_unix_time_from_hhmm_time
 * @param time
 * @return
 */
time_t	build_unix_time_from_hhmm_time(const std::string& time);

#endif // CONVERTIONS_H
