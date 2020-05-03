#include "document.hpp"
#include "parser.hpp"

using namespace cssdom;

namespace{
struct dom_parser : public parser{
	document doc;

	virtual void on_selector_tag(std::string&& str)override{
		TRACE(<< "selector tag: " << str << std::endl)
	}

	virtual void on_selector_class(std::string&& str)override{
		TRACE(<< "selector class: " << str << std::endl)
	}

	virtual void on_combinator(std::string&& str)override{
		TRACE(<< "combinator: " << str << std::endl)
	}

	virtual void on_style_properties_start()override{
		TRACE(<< "style properties START" << std::endl)
	}

	virtual void on_style_properties_end()override{
		TRACE(<< "style properties END" << std::endl)
	}

	virtual void on_property_name(std::string&& str)override{
		TRACE(<< "property name: " << str << std::endl)
	}

	virtual void on_property_value(std::string&& str)override{
		TRACE(<< "property value: " << str << std::endl)
	}
};

}

document cssdom::read(const papki::file& fi){

	dom_parser p;
	
	{
		papki::file::guard file_guard(fi);

		std::array<uint8_t, 4096> buf; // 4k

		while(true){
			auto res = fi.read(utki::make_span(buf));
			ASSERT_ALWAYS(res <= buf.size())
			if(res == 0){
				break;
			}
			p.feed(utki::make_span(buf.data(), res));
		}
	}
	
	return p.doc;

}
