#include "../../src/cssdom/dom.hpp"

#include <map>

#include <papki/span_file.hpp>

#include <utki/tree.hpp>

namespace{
class dom_node : public cssdom::styleable{
public:
	std::string id;
	std::string tag;

	std::vector<std::string> classes;

	dom_node(std::string&& tag, std::string&& id, std::vector<std::string>&& classes = std::vector<std::string>()) :
			id(id),
			tag(tag),
			classes(classes)
	{}

	dom_node(std::string&& tag, std::vector<std::string>&& classes = std::vector<std::string>()) :
			tag(tag),
			classes(classes)
	{}

	const std::string& get_id()const override{
		return this->id;
	}

	const std::string& get_tag()const override{
		return this->tag;
	}

	utki::span<const std::string> get_classes()const override{
		return utki::make_span(this->classes);
	}
};
}

namespace{
enum class property_id{
	fill,
	stroke,
	stroke_width
};
std::map<std::string, uint32_t> property_name_to_id_map{
	{"fill", uint32_t(property_id::fill)},
	
};
}

namespace{
struct property_value : public utki::destructable{
	std::string value;
};
}

int main(int argc, char** argv){
	// test basic property vaslue query
	{
		auto css = utki::make_span(R"qwertyuiop(
			rect {
				fill: red;
				stroke: blue;
				stroke-width: 3
			}
		)qwertyuiop");

		auto css_dom = cssdom::read(
				papki::span_file(utki::make_span(reinterpret_cast<const uint8_t*>(css.data()), css.size())),
				property_name_to_id_map,
				[](uint32_t id, std::string&& v) -> std::unique_ptr<utki::destructable> {
					auto ret = std::make_unique<property_value>();
					ret->value = std::move(v);
					return ret;
				}
			);

		typedef utki::tree<dom_node> node;
		node::container_type dom{
			node(dom_node("body"), {
				node(dom_node("rect"))
			})
		};

		// TODO:
	}

	return 0;
}
