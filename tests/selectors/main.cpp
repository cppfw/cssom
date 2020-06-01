#include "../../src/cssdom/dom.hpp"

#include <map>

#include <papki/span_file.hpp>

#include <utki/tree.hpp>

#include "../harness/properties.hpp"
#include "../harness/dom.hpp"

int main(int argc, char** argv){
	// test basic property value query
	{
		auto css = R"qwertyuiop(
			rect {
				fill: red;
				stroke: blue;
				stroke-width: 3
			}
			circle.myGreen {
				stroke: #006600; fill: #00cc00;
			}
			circle.myRed {
				stroke: #670000; fill: #cc0000;
			}
		)qwertyuiop";

		auto pntim = &property_name_to_id_map;

		const auto css_dom = cssdom::read(
				papki::span_file(utki::make_span(css)),
				[pntim](const std::string& name) -> uint32_t{
					auto i = pntim->find(name);
					if(i == pntim->end()){
						return uint32_t(property_id::ENUM_SIZE);
					}
					return uint32_t(i->second);
				},
				[](uint32_t id, std::string&& v) -> std::unique_ptr<cssdom::property_value_base> {
					auto ret = std::make_unique<property_value>(std::move(v));
					return ret;
				}
			);

		typedef utki::tree<dom_node> node;
		node::container_type dom{
			node(dom_node("body"), {
				node(dom_node("rect")),
				node(dom_node("circle", std::string(), {"myGreen"}))
			})
		};

		{
			crawler cr(dom, {0, 0});
			
			auto qr = css_dom.get_property_value(cr, uint32_t(property_id::stroke));
			ASSERT_ALWAYS(qr.value);

			ASSERT_ALWAYS(dynamic_cast<const property_value*>(qr.value))

			auto pv = static_cast<const property_value*>(qr.value);
			ASSERT_ALWAYS(pv->value == "blue")
		}
		{
			crawler cr(dom, {0, 1});
			
			auto qr = css_dom.get_property_value(cr, uint32_t(property_id::stroke));
			ASSERT_ALWAYS(qr.value);

			ASSERT_ALWAYS(dynamic_cast<const property_value*>(qr.value))

			auto pv = static_cast<const property_value*>(qr.value);
			ASSERT_ALWAYS(pv->value == "#006600")
		}
	}

	return 0;
}
