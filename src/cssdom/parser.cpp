#include "parser.hpp"

#include <sstream>

using namespace cssdom;

void parser::feed(const utki::span<char> data){
	for(auto i = data.begin(), e = data.end(); i != e; ++i){
		switch(this->cur_state){
			case state::idle:
				this->parse_idle(i, e);
				break;
			case state::selector_tag:
				this->parse_selector_tag(i, e);
				break;
			case state::selector_class:
				this->parse_selector_class(i, e);
				break;
			case state::combinator:
				this->parse_combinator(i, e);
				break;
		}
		if(i == e){
			return;
		}
	}
}

void parser::parse_idle(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e){
	for(; i != e; ++i){
		ASSERT(this->buf.empty())
		switch(*i){
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				break;
			case '.':
				this->cur_state = state::selector_class;
				return;
			case '[':
				ASSERT_INFO(false, "parsing of attribute selectors is not implemented")
				break;
			default:
				this->buf.push_back(*i);
				this->cur_state = state::selector_tag;
				return;
		}
	}
}

void parser::parse_selector_tag(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e){
	for(; i != e; ++i){
		ASSERT(!this->buf.empty())
		switch(*i){
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				this->on_selector_tag(std::string(this->buf.data(), this->buf.size()));
				this->buf.clear();
				this->cur_state = state::combinator;
				return;
			// case '.':
			// 	this->cur_state = state::selector_class;
			// 	return;
			// case '[':
			// 	ASSERT_INFO(false, "parsing of attribute selectors is not implemented")
			// 	break;
			default:
				this->buf.push_back(*i);
				return;
		}
	}
}

void parser::parse_selector_class(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e){
	for(; i != e; ++i){
		// ASSERT(this->buf.empty())
		switch(*i){
			// case '\n':
			//	++this->line;
			// case ' ':
			// case '\r':
			// case '\t':
			// 	break;
			// case '.':
			// 	this->cur_state = state::selector_class;
			// 	return;
			// case '[':
			// 	ASSERT_INFO(false, "parsing of attribute selectors is not implemented")
			// 	break;
			// default:
			// 	this->buf.push_back(*i);
			// 	this->cur_state = state::selector_tag;
			// 	return;
		}
	}
}

void parser::parse_combinator(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e){
	for(; i != e; ++i){
		// ASSERT(this->buf.empty())
		switch(*i){
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				if(this->buf.empty()){
					break;
				}
				this->on_combinator(std::string(this->buf.data(), this->buf.size()));
				this->buf.clear();
				this->cur_state = state::idle;
				break;
			case '>':
			case '+':
			case '~':
				if(!this->buf.empty()){
					std::stringstream ss;
					ss << "unknown combinator encountered (" << std::string(this->buf.data(), this->buf.size()) << ") at line " << this->line;
					throw malformed_css_error(ss.str());
				}
				this->buf.push_back(*i);
				break;
			default:
				// TODO:
				break;
		}
	}
}
