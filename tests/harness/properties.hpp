#pragma once

#include <map>
#include <string>

#include <utki/destructable.hpp>

#include "../../src/cssdom/dom.hpp"

enum class property_id{
	fill,
	stroke,
	stroke_width,
	background_color,
	background_image
};

extern std::map<std::string, uint32_t> property_name_to_id_map;

struct property_value : public cssdom::property_value_base{
	std::string value;

	property_value(std::string&& value) : value(std::move(value)) {}
};
