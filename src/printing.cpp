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
