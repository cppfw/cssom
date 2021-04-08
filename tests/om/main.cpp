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

		auto css_om1 = read_css(css1);
		auto css_om2 = read_css(css2);

		css_om1.append(std::move(css_om2));

		utki::assert(css_om1.styles.size() == 2, SL);
		utki::assert(css_om1.styles.front().specificity == 2, [&](auto&o){o << "specificity = " << css_om1.styles.front().specificity;}, SL);
		utki::assert(css_om1.styles.back().specificity == 1, [&](auto&o){o << "specificity = " << css_om1.styles.back().specificity;}, SL);
	}

	return 0;
}
