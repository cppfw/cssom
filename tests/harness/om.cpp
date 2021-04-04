#include "om.hpp"
#include "properties.hpp"

#include <papki/span_file.hpp>

cssom::document read_css(const char* css){
	auto pntim = &property_name_to_id_map;

	return cssom::read(
			papki::span_file(utki::make_span(css)),
			[pntim](const std::string& name) -> uint32_t{
				auto i = pntim->find(name);
				if(i == pntim->end()){
					return uint32_t(property_id::ENUM_SIZE);
				}
				return uint32_t(i->second);
			},
			[](uint32_t id, std::string&& v) -> std::unique_ptr<cssom::property_value_base> {
				auto ret = std::make_unique<property_value>(std::move(v));
				return ret;
			}
		);
}
