#include <tst/set.hpp>
#include <tst/check.hpp>

#include "../../src/cssom/om.hpp"

#include <papki/span_file.hpp>

#include "../harness/properties.hpp"
#include "../harness/om.hpp"

namespace{
tst::set set("object_model", [](auto& suite){
	suite.add(
		"append",
		[](){
			auto css1 = R"qwertyuiop(
				rect {
					fill: red;
					stroke: blue;
					stroke-width: 3
				}
			)qwertyuiop";

			auto css2 = R"qwertyuiop(
				body rect {
					fill: red;
					stroke: blue;
					stroke-width: 3
				}
			)qwertyuiop";

			auto css_om1 = read_css(css1);
			auto css_om2 = read_css(css2);

			css_om1.append(std::move(css_om2));

			tst::check_eq(css_om1.styles.size(), size_t(2), SL);
			tst::check_eq(css_om1.styles.front().specificity, unsigned(2), SL) << "specificity = " << css_om1.styles.front().specificity;
			tst::check_eq(css_om1.styles.back().specificity, unsigned(1), SL) << "specificity = " << css_om1.styles.back().specificity;
		}
	);
});
}
