#include "properties.hpp"

std::map<std::string, uint32_t> property_name_to_id_map{
	{"fill", uint32_t(property_id::fill)},
	{"stroke", uint32_t(property_id::stroke)},
	{"background-color", uint32_t(property_id::background_color)},
	{"background-image", uint32_t(property_id::background_image)}
};
