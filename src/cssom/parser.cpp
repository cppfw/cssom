/*
MIT License

Copyright (c) 2020-2022 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/* ================ LICENSE END ================ */

#include "parser.hpp"

#include <sstream>

#include <utki/string.hpp>

#ifdef assert
#	undef assert
#endif

using namespace cssom;

void parser::feed(utki::span<const char> data)
{
	for (auto i = data.begin(), e = data.end(); i != e; ++i) {
		switch (this->cur_state) {
			case state::idle:
				this->parse_idle(i, e);
				break;
			case state::style_idle:
				this->parse_style_idle(i, e);
				break;
			case state::selector_tag:
				this->parse_selector_tag(i, e);
				break;
			case state::selector_id:
				this->parse_selector_id(i, e);
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
		if (i == e) {
			return;
		}
	}
}

void parser::parse_idle(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e)
{
	for (; i != e; ++i) {
		ASSERT(this->buf.empty())
		switch (*i) {
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				break;
			case '.':
				this->cur_state = state::selector_class;
				return;
			case '#':
				this->cur_state = state::selector_id;
				return;
			case '[':
				throw std::runtime_error("parsing of attribute selectors is not implemented");
				break;
			default:
				this->buf.push_back(*i);
				this->cur_state = state::selector_tag;
				return;
		}
	}
}

void parser::parse_style_idle(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e)
{
	for (; i != e; ++i) {
		ASSERT(this->buf.empty())
		switch (*i) {
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

void parser::notify_selector_tag()
{
	this->on_selector_tag(utki::make_string(utki::make_span(this->buf)));
	this->buf.clear();
	this->on_selector_end();
}

void parser::parse_selector_tag(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e)
{
	for (; i != e; ++i) {
		ASSERT(!this->buf.empty())
		switch (*i) {
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				this->notify_selector_tag();
				this->cur_state = state::combinator;
				return;
			case '{':
				this->notify_selector_tag();
				this->on_selector_chain_end();
				this->cur_state = state::style_idle;
				return;
			case ',':
				this->notify_selector_tag();
				this->on_selector_chain_end();
				this->cur_state = state::idle;
				return;
			case '.':
				this->on_selector_tag(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->cur_state = state::selector_class;
				return;
			case '#':
				this->on_selector_tag(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->cur_state = state::selector_id;
				return;
			case '[':
				throw std::runtime_error("parsing of attribute selectors is not implemented");
				break;
			default:
				this->buf.push_back(*i);
				break;
		}
	}
}

void parser::notify_selector_id()
{
	this->on_selector_id(utki::make_string(utki::make_span(this->buf)));
	this->buf.clear();
	this->on_selector_end();
}

void parser::parse_selector_id(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e)
{
	for (; i != e; ++i) {
		switch (*i) {
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				this->notify_selector_id();
				this->cur_state = state::combinator;
				return;
			case '{':
				this->notify_selector_id();
				this->on_selector_chain_end();
				this->cur_state = state::style_idle;
				return;
			case ',':
				this->notify_selector_id();
				this->on_selector_chain_end();
				this->cur_state = state::idle;
				return;
			case '.':
				this->on_selector_id(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->cur_state = state::selector_class;
				return;
			case '#':
				{
					std::stringstream ss;
					ss << "unexpected # encountered at line " << this->line;
					throw malformed_css_error(ss.str());
				}
			case '[':
				throw std::runtime_error("parsing of attribute selectors is not implemented");
				break;
			default:
				this->buf.push_back(*i);
				break;
		}
	}
}

void parser::notify_selector_class()
{
	this->on_selector_class(utki::make_string(utki::make_span(this->buf)));
	this->buf.clear();
	this->on_selector_end();
}

void parser::parse_selector_class(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e)
{
	for (; i != e; ++i) {
		switch (*i) {
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				this->notify_selector_class();
				this->cur_state = state::combinator;
				return;
			case '.':
				this->on_selector_class(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				break;
			case '[':
				throw std::runtime_error("parsing of attribute selectors is not implemented");
				break;
			case '#':
				this->on_selector_tag(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->cur_state = state::selector_id;
				return;
			case '{':
				this->notify_selector_class();
				this->on_selector_chain_end();
				this->cur_state = state::style_idle;
				return;
			case ',':
				this->notify_selector_class();
				this->on_selector_chain_end();
				this->cur_state = state::idle;
				return;
			default:
				this->buf.push_back(*i);
				break;
		}
	}
}

void parser::parse_combinator(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e)
{
	for (; i != e; ++i) {
		switch (*i) {
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				if (this->buf.empty()) {
					break;
				}
				this->on_combinator(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();
				this->cur_state = state::idle;
				return;
			case '>':
			case '+':
			case '~':
				if (!this->buf.empty()) {
					std::stringstream ss;
					ss << "unknown combinator encountered (" << utki::make_string(utki::make_span(this->buf)) << *i
					   << ") at line " << this->line;
					throw malformed_css_error(ss.str());
				}
				this->buf.push_back(*i);
				break;
			case '{':
				if (!this->buf.empty()) {
					std::stringstream ss;
					ss << "unexpected combinator encountered (" << utki::make_string(utki::make_span(this->buf))
					   << ") at line " << this->line;
					throw malformed_css_error(ss.str());
				}
				this->on_selector_chain_end();
				this->cur_state = state::style_idle;
				return;
			case ',':
				if (!this->buf.empty()) {
					std::stringstream ss;
					ss << "unexpected combinator encountered (" << utki::make_string(utki::make_span(this->buf))
					   << ") at line " << this->line;
					throw malformed_css_error(ss.str());
				}
				this->on_selector_chain_end();
				this->cur_state = state::idle;
				return;
			default:
				this->on_combinator(utki::make_string(utki::make_span(this->buf)));
				this->buf.clear();

				switch (*i) {
					case '.':
						this->cur_state = state::selector_class;
						break;
					case '[':
						utki::assert(
							false,
							[](auto& o) {
								o << "attribute selectors are not implemented";
							},
							SL
						);
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

void parser::parse_property_name(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e)
{
	for (; i != e; ++i) {
		switch (*i) {
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				if (this->buf.empty()) {
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

void parser::parse_property_value_delimiter(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e)
{
	for (; i != e; ++i) {
		ASSERT(this->buf.empty())
		switch (*i) {
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

void parser::parse_property_value(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e)
{
	for (; i != e; ++i) {
		switch (*i) {
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				if (this->buf.empty()) {
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

void parser::parse_property_value_terminator(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e)
{
	for (; i != e; ++i) {
		ASSERT(this->buf.empty())
		switch (*i) {
			case '\n':
				++this->line;
			case ' ':
			case '\r':
			case '\t':
				break;
			case ';':
				this->cur_state = state::style_idle;
				return;
			case '}':
				this->on_style_properties_end();
				this->cur_state = state::idle;
				return;
			default:
				std::stringstream ss;
				ss << "unexpected character (" << *i << ") after style property value at line " << this->line;
				throw malformed_css_error(ss.str());
		}
	}
}
