#pragma once

#include <papki/file.hpp>

namespace cssdom{

struct document{
	std::vector<std::string> stuff;
};

document read(const papki::file& fi);

}
