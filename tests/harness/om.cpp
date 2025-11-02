#include "om.hpp"
#include "properties.hpp"

#include <fsif/span_file.hpp>

cssom::sheet read_css(const char* css){
	auto pntim = &property_name_to_id_map;

	return cssom::read(
			fsif::span_file(utki::make_span(css)),
			[pntim](std::string_view name) -> uint32_t{
				auto i = pntim->find(name);
				if(i == pntim->end()){
					return uint32_t(property_id::enum_size);
				}
				return uint32_t(i->second);
			},
			[](uint32_t id, std::string_view v) -> std::unique_ptr<cssom::property_value_base> {
				auto ret = std::make_unique<property_value>(std::string(v));
				return ret;
			}
		);
}
