#pragma once

#include <utki/span.hpp>

namespace cssdom{

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

	void parse_idle(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e);
	void parse_style_idle(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e);
	void parse_selector_tag(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e);
	void parse_selector_class(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e);
	void parse_combinator(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e);
	void parse_property_name(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e);
	void parse_property_value_delimiter(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e);
	void parse_property_value(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e);
	void parse_property_value_terminator(utki::span<char>::const_iterator& i, utki::span<char>::const_iterator& e);
public:
	parser() = default;

	virtual ~parser()noexcept{}

	virtual void on_selector_start() = 0;
	virtual void on_selector_end() = 0;
	virtual void on_selector_tag(std::string&& str) = 0;
	virtual void on_selector_class(std::string&& str) = 0;
	virtual void on_combinator(std::string&& str) = 0;
	virtual void on_style_properties_start() = 0;
	virtual void on_style_properties_end() = 0;
	virtual void on_property_name(std::string&& str) = 0;
	virtual void on_property_value(std::string&& str) = 0;

	/**
	 * @brief feed UTF-8 data to parser.
	 * @param data - data to be fed to parser.
	 */
	void feed(const utki::span<char> data);
	
	/**
	 * @brief feed UTF-8 data to parser.
	 * @param data - data to be fed to parser.
	 */
	void feed(const utki::span<uint8_t> data){
		this->feed(utki::make_span(reinterpret_cast<const char*>(&*data.begin()), data.size()));
	}
	
	/**
	 * @brief Parse in string.
	 * @param str - string to parse.
	 */
	void feed(const std::string& str){
		this->feed(utki::make_span(str.c_str(), str.length()));
	}
	
};

}
