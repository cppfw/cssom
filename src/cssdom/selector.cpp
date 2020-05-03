#include "selector.hpp"

#include <sstream>

using namespace cssdom;

combinator cssdom::parse_combinator(const std::string& str){
	if(str.empty()){
		return combinator::descendant;
	}else if(str == ">"){
		return combinator::child;
	}else if(str == "+"){
		return combinator::next_sibling;
	}else if(str == "~"){
		return combinator::subsequent_sibling;
	}
	std::stringstream ss;
	ss << "unknown combinator: " << str;
	throw std::logic_error(ss.str());
}

unsigned selector::calculate_specificity()const noexcept{
	//TODO:
	return 0;
}
