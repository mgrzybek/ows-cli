#include <string>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

#include "model_types.h"

void	update_node(const std::string& key, const std::string& value, rpc::t_node node);
bool	split_line(const char& separator, const std::string& data, std::string& key, std::string& value);
