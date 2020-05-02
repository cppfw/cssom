#pragma once

#include <string>
#include <vector>

namespace cssdom{

struct selector{

	std::string tag;

	std::vector<std::string> classes;

	// TODO: attribute selectors, pseido-class, pseudo-element etc.
};

}
