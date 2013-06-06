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
