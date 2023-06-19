#include <tst/set.hpp>
#include <tst/check.hpp>

#include <cssom/om.hpp>

#include "../harness/properties.hpp"
#include "../harness/om.hpp"

namespace{
const tst::set set("misc", [](tst::suite& suite){
    suite.add("issue_test_1", [](){
        auto css_str = ".cls-1,.cls-3{fill:none;}.cls-2,.cls-4{fill:#47506a;}.cls-3{stroke:#3866f0;stroke-width:4px;}.cls-4{fill-rule:evenodd;}";
        auto css = read_css(css_str);
        tst::check_eq(css.styles.size(), size_t(6), SL);

        using node = utki::tree<om_node>;
        node::container_type dom{
            node(om_node("body"), {
                node(om_node("rect", "rect-2", {"cls-2"})),
                node(om_node("circle", std::string(), {"myGreen"}))
            })
        };

        crawler cr(dom, {0, 0});
                
        auto qr = css.get_property_value(cr, uint32_t(property_id::fill));
        tst::check(qr.value, SL);
    });
});
}
