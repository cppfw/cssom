#include "parser.hpp"

#include <sstream>

#include <utki/string.hpp>

using namespace cssdom;

void parser::feed(utki::span<const char> data){
	for(auto i = data.begin(), e = data.end(); i != e; ++i){
		switch(this->cur_state){
			case state::idle:
				this->parse_idle(i, e);
				break;
			case state::style_idle:
				this->parse_style_idle(i, e);
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
			case state::property_name:
				this->parse_property_name(i, e);
				break;
			case state::property_value_delimiter:
				this->parse_property_value_delimiter(i, e);
				break;
			case state::property_value:
				this->parse_property_value(i, e);
				break;
			case state::property_value_terminator:
				this->parse_property_value_terminator(i, e);
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

void parser::parse_style_idle(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e){
	for(; i != e; ++i){
		ASSERT(this->buf.empty())
		switch(*i){
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				break;
			case '}':
				this->on_style_properties_end();
				this->cur_state = state::idle;
				return;
			default:
				this->buf.push_back(*i);
				this->cur_state = state::property_name;
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
				this->on_selector_tag(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->on_selector_end();
				this->cur_state = state::combinator;
				return;
			case '{':
				this->on_selector_tag(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->on_selector_end();
				this->on_selector_chain_end();
				this->cur_state = state::style_idle;
				return;
			case '.':
				this->on_selector_tag(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->cur_state = state::selector_class;
				return;
			case '[':
				ASSERT_INFO(false, "parsing of attribute selectors is not implemented")
				break;
			default:
				this->buf.push_back(*i);
				break;
		}
	}
}

void parser::parse_selector_class(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e){
	for(; i != e; ++i){
		switch(*i){
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				this->on_selector_class(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->on_selector_end();
				this->cur_state = state::combinator;
				return;
			case '.':
				this->on_selector_class(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				break;
			case '[':
				ASSERT_INFO(false, "parsing of attribute selectors is not implemented")
				break;
			case '{':
				this->on_selector_class(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->on_selector_end();
				this->on_selector_chain_end();
				this->cur_state = state::style_idle;
				return;
			default:
				this->buf.push_back(*i);
				break;
		}
	}
}

void parser::parse_combinator(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e){
	for(; i != e; ++i){
		switch(*i){
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				if(this->buf.empty()){
					break;
				}
				this->on_combinator(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->cur_state = state::idle;
				return;
			case '>':
			case '+':
			case '~':
				if(!this->buf.empty()){
					std::stringstream ss;
					ss << "unknown combinator encountered (" << utki::make_string(utki::make_span(this->buf)) << *i << ") at line " << this->line;
					throw malformed_css_error(ss.str());
				}
				this->buf.push_back(*i);
				break;
			case '{':
				if(!this->buf.empty()){
					std::stringstream ss;
					ss << "unexpected combinator encountered (" << utki::make_string(utki::make_span(this->buf)) << ") at line " << this->line;
					throw malformed_css_error(ss.str());
				}
				this->on_selector_chain_end();
				this->cur_state = state::style_idle;
				return;
			default:
				this->on_combinator(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();

				switch(*i){
					case '.':
						this->cur_state = state::selector_class;
						break;
					case '[':
						ASSERT_INFO_ALWAYS(false, "attribute selectors are not implemented")
						break;
					default:
						this->buf.push_back(*i);
						this->cur_state = state::selector_tag;
						break;
				}
				
				return;
		}
	}
}

void parser::parse_property_name(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e){
	for(; i != e; ++i){
		switch(*i){
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				if(this->buf.empty()){
					break;
				}
				this->on_property_name(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->cur_state = state::property_value_delimiter;
				return;
			case ':':
				this->on_property_name(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->cur_state = state::property_value;
				return;
			default:
				this->buf.push_back(*i);
				break;
		}
	}
}

void parser::parse_property_value_delimiter(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e){
	for(; i != e; ++i){
		ASSERT(this->buf.empty())
		switch(*i){
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				break;
			case ':':
				this->cur_state = state::property_value;
				return;
			default:
				std::stringstream ss;
				ss << "unexpected characters after style property name at line " << this->line;
				throw malformed_css_error(ss.str());
		}
	}
}

void parser::parse_property_value(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e){
	for(; i != e; ++i){
		switch(*i){
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				if(this->buf.empty()){
					break;
				}
				this->on_property_value(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->cur_state = state::property_value_terminator;
				return;
			case ';':
				this->on_property_value(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->cur_state = state::style_idle;
				return;
			default:
				this->buf.push_back(*i);
				break;
		}
	}
}

void parser::parse_property_value_terminator(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e){
	for(; i != e; ++i){
		ASSERT(this->buf.empty())
		switch(*i){
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				break;
			case ';':
				this->cur_state = state::style_idle;
				return;
			default:
				std::stringstream ss;
				ss << "unexpected characters after style property value at line " << this->line;
				throw malformed_css_error(ss.str());
		}
	}
}
