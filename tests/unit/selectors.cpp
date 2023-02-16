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

            const auto css_dom = read_css(css);

            using node = utki::tree<om_node>;
            node::container_type dom{
                node(om_node("body"), {
                    node(om_node("rect")),
                    node(om_node("circle", std::string(), {"myGreen"}))
                })
            };

            {
                crawler cr(dom, {0, 0});
                
                auto qr = css_dom.get_property_value(cr, uint32_t(property_id::stroke));
                tst::check(qr.value, SL);

                tst::check(dynamic_cast<const property_value*>(qr.value), SL);

                auto pv = static_cast<const property_value*>(qr.value);
                tst::check(pv->value == "blue", SL);
            }
            {
                crawler cr(dom, {0, 1});
                
                auto qr = css_dom.get_property_value(cr, uint32_t(property_id::stroke));
                tst::check(qr.value, SL);

                tst::check(dynamic_cast<const property_value*>(qr.value), SL);

                auto pv = static_cast<const property_value*>(qr.value);
                tst::check(pv->value == "#006600", SL);
            }
        }
    );
});
}
