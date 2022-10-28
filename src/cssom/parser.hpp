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

#pragma once

#include <utki/span.hpp>

namespace cssom{

class malformed_css_error : public std::logic_error{
public:
	malformed_css_error(const std::string& message) :
			std::logic_error(message)
	{}
};

class parser{
	uint32_t line = 0;

	enum class state{
		idle,
		style_idle, // idle state inside style block
		selector_tag,
		selector_class,
		combinator,
		property_name,
		property_value_delimiter, // colon between property name and value
		property_value,
		property_value_terminator // semicolon
	};

	state cur_state = state::idle;

	std::vector<char> buf;

	void parse_idle(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e);
	void parse_style_idle(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e);
	void parse_selector_tag(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e);
	void parse_selector_class(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e);
	void parse_combinator(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e);
	void parse_property_name(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e);
	void parse_property_value_delimiter(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e);
	void parse_property_value(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e);
	void parse_property_value_terminator(utki::span<const char>::iterator& i, utki::span<const char>::iterator& e);
public:
	parser() = default;

	virtual ~parser()noexcept{}

	virtual void on_selector_chain_end() = 0;
	virtual void on_selector_end() = 0;
	virtual void on_selector_tag(std::string&& str) = 0;
	virtual void on_selector_class(std::string&& str) = 0;
	virtual void on_combinator(std::string&& str) = 0;
	virtual void on_style_properties_end() = 0;
	virtual void on_property_name(std::string&& str) = 0;
	virtual void on_property_value(std::string&& str) = 0;

	/**
	 * @brief feed UTF-8 data to parser.
	 * @param data - data to be fed to parser.
	 */
	void feed(utki::span<const char> data);
	
	/**
	 * @brief feed UTF-8 data to parser.
	 * @param data - data to be fed to parser.
	 */
	void feed(const utki::span<uint8_t> data){
		this->feed(utki::make_span(reinterpret_cast<const char*>(data.data()), data.size()));
	}
	
	/**
	 * @brief feed UTF-8 string to parser.
	 * @param str - string to be fed parser.
	 */
	void feed(const std::string& str){
		this->feed(utki::make_span(str.c_str(), str.length()));
	}
	
};

}
