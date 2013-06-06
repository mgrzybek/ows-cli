#ifndef _PRINTING_H_
#include <iostream>
#include <algorithm>
#include <boost/foreach.hpp>

#include "convertions.h"
#include "model_types.h"

void	print_node(const rpc::t_node& node);

void	print_jobs(const rpc::v_jobs& jobs);
void	print_job(const rpc::t_job& job);

#endif // _PRINTING_H_
