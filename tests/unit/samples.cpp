#include <tst/set.hpp>
#include <tst/check.hpp>

#include <utki/util.hpp>

#include <papki/fs_file.hpp>
#include <papki/vector_file.hpp>

#include <regex>

#include "../harness/properties.hpp"

namespace{
const std::string data_dir = "samples_data/";
}

namespace{
// NOLINTNEXTLINE(cppcoreguidelines-interfaces-global-init)
const tst::set set("samples", [](tst::suite& suite){
    std::vector<std::string> files;

    {
		const std::regex suffix_regex("^.*\\.css$");
		auto all_files = papki::fs_file(data_dir).list_dir();

		std::copy_if(
				all_files.begin(),
				all_files.end(),
				std::back_inserter(files),
				[&suffix_regex](auto& f){
					return std::regex_match(f, suffix_regex);
				}
			);
	}

    suite.add<std::string>(
        "sample",
        std::move(files),
        [](auto& p){
            auto in_file_name = data_dir + p;

            auto doc = cssom::read(
                    papki::fs_file(in_file_name),
                    [](std::string_view name) -> uint32_t{
                        auto i = property_name_to_id_map.find(name);
                        if(i == property_name_to_id_map.end()){
                            return uint32_t(property_id::enum_size);
                        }
                        return uint32_t(i->second);
                    },
                    [](uint32_t id, std::string_view value) -> std::unique_ptr<cssom::property_value_base>{
                        return std::make_unique<property_value>(std::string(value));
                    }
                );
            
            auto pitnm = utki::flip_map(property_name_to_id_map);

            papki::vector_file out_file;

            doc.write(
                    out_file,
                    [&pitnm](uint32_t id) -> std::string{
                        auto i = pitnm.find(id);
                        if(i == pitnm.end()){
                            return {};
                        }
                        return i->second;
                    },
                    [](uint32_t id, const cssom::property_value_base& value) -> std::string{
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                        auto& v = static_cast<const property_value&>(value);
                        return v.value;
                    }
                );
            
            auto out_data = out_file.reset_data();

            decltype(out_data) cmp_data;

            papki::fs_file cmp_file(in_file_name + ".cmp");

            try{
                cmp_data = cmp_file.load();
            }catch(std::system_error& e){
                std::cout << "could not load file: " << cmp_file.path() << std::endl;
            }

            if(out_data != cmp_data){
                papki::fs_file failed_file(data_dir + p + ".out");

                failed_file.open(papki::file::mode::create);
                failed_file.write(out_data);
                failed_file.close();

                tst::check(false, SL) << "parsed file is not as expected: " << in_file_name;
            }
        }
    );
});
}
