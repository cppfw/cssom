#pragma once

#include <string>
#include <vector>

namespace cssdom{

enum class combinator{
	descendant,
	child,
	next_sibling,
	subsequent_sibling
};

combinator parse_combinator(const std::string& str);

struct selector{

	std::string tag;

	std::vector<std::string> classes;

	// TODO: attribute selectors, pseido-class, pseudo-element etc.

	cssdom::combinator combinator;

	unsigned calculate_specificity()const noexcept;
};

}
