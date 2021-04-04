#include "../../src/cssom/om.hpp"

#include <papki/span_file.hpp>

#include "../harness/properties.hpp"
#include "../harness/om.hpp"

int main(int argc, char** argv){
	// test append
	{
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

		auto css_dom1 = read_css(css1);
		auto css_dom2 = read_css(css2);

		css_dom1.append(std::move(css_dom2));

		ASSERT_ALWAYS(css_dom1.styles.size() == 2)
		ASSERT_INFO_ALWAYS(css_dom1.styles.front().specificity == 2, "specificity = " << css_dom1.styles.front().specificity)
		ASSERT_INFO_ALWAYS(css_dom1.styles.back().specificity == 1, "specificity = " << css_dom1.styles.back().specificity)
	}

	return 0;
}
