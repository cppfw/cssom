#include "../../src/cssom/om.hpp"

#include <clargs/parser.hpp>
#include <papki/fs_file.hpp>

#include <memory>

#include "../harness/properties.hpp"

int main(int argc, char** argv){
	bool help = false;
	std::string out_filename = "out.css";
	std::string in_filename;

	{
		clargs::parser p;

		p.add("help", "show help information", [&help](){help = true;});
		p.add('o', "out-file", "output filename", [&out_filename](std::string&& f){out_filename = std::move(f);});

		auto extras = p.parse(argc, argv);

		if(help){
			std::cout << "CSS parser test utility. Parses CSS into DOM and then writes out the DOM to CSS file." << std::endl;
			std::cout << "Usage:" << std::endl;
			std::cout << "  " << argv[0] << " [-o <out-css-filename>] <in-css-filename>" << std::endl;
			std::cout << "Arguments:" << std::endl;
			std::cout << p.description() << std::endl;
			return 0;
		}

		if(extras.empty()){
			std::cout << "error: no input filename given" << std::endl;
			return 1;
		}
		in_filename = extras.front();
	}

	auto doc = cssom::read(
			papki::fs_file(in_filename),
			[](const std::string& name) -> uint32_t{
				auto i = property_name_to_id_map.find(name);
				if(i == property_name_to_id_map.end()){
					return uint32_t(property_id::ENUM_SIZE);
				}
				return uint32_t(i->second);
			},
			[](uint32_t id, std::string&& value) -> std::unique_ptr<cssom::property_value_base>{
				return std::make_unique<property_value>(std::move(value));
			}
		);
	
	auto pitnm = utki::flip_map(property_name_to_id_map);

	doc.write(
			papki::fs_file(out_filename),
			[&pitnm](uint32_t id) -> std::string{
				auto i = pitnm.find(id);
				if(i == pitnm.end()){
					return std::string();
				}
				return i->second;
			},
			[](uint32_t id, const cssom::property_value_base& value) -> std::string{
				auto& v = static_cast<const property_value&>(value);
				return v.value;
			}
		);

	return 0;
}
