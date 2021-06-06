#include <tst/set.hpp>
#include <tst/check.hpp>

#include <papki/span_file.hpp>

#include "../harness/properties.hpp"
#include "../harness/om.hpp"

namespace{
tst::set set("selectors", [](auto& suite){
    suite.add(
        "basic_property_value_query",
        [](){
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

            const auto css_dom = cssom::read(
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

            typedef utki::tree<om_node> node;
            node::container_type dom{
                node(om_node("body"), {
                    node(om_node("rect")),
                    node(om_node("circle", std::string(), {"myGreen"}))
                })
            };

            {
                crawler cr(dom, {0, 0});
                
                auto qr = css_dom.get_property_value(cr, uint32_t(property_id::stroke));
                utki::assert(qr.value, SL);

                utki::assert(dynamic_cast<const property_value*>(qr.value), SL);

                auto pv = static_cast<const property_value*>(qr.value);
                utki::assert(pv->value == "blue", SL);
            }
            {
                crawler cr(dom, {0, 1});
                
                auto qr = css_dom.get_property_value(cr, uint32_t(property_id::stroke));
                utki::assert(qr.value, SL);

                utki::assert(dynamic_cast<const property_value*>(qr.value), SL);

                auto pv = static_cast<const property_value*>(qr.value);
                utki::assert(pv->value == "#006600", SL);
            }
        }
    );
});
}
