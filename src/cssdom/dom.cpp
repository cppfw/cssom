#include "dom.hpp"
#include "parser.hpp"

using namespace cssdom;

namespace{
struct dom_parser : public parser{
	document doc;

	const std::map<std::string, uint32_t>& property_name_to_id_map;
	const std::function<std::unique_ptr<utki::destructable>(uint32_t, std::string&&)>& parse_property;

	dom_parser(
			const std::map<std::string, uint32_t>& property_name_to_id_map,
			const std::function<std::unique_ptr<utki::destructable>(uint32_t, std::string&&)>& parse_property
		) :
			property_name_to_id_map(property_name_to_id_map),
			parse_property(parse_property)
	{}

	virtual void on_selector_start()override{
		TRACE(<< "selector START" << std::endl)
	}

	virtual void on_selector_end()override{
		TRACE(<< "selector END" << std::endl)
	}

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

document cssdom::read(
		const papki::file& fi,
		const std::map<std::string, uint32_t>& property_name_to_id_map,
		const std::function<std::unique_ptr<utki::destructable>(uint32_t, std::string&&)>& parse_property
	)
{
	if(!parse_property){
		throw std::logic_error("cssdom::read(): passed in 'parse_property' function is nullptr");
	}

	dom_parser p(property_name_to_id_map, parse_property);
	
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
	
	return std::move(p.doc);

}

combinator cssdom::parse_combinator(const std::string& str){
	if(str.empty()){
		return combinator::descendant;
	}else if(str == ">"){
		return combinator::child;
	}else if(str == "+"){
		return combinator::next_sibling;
	}else if(str == "~"){
		return combinator::subsequent_sibling;
	}
	std::stringstream ss;
	ss << "unknown combinator: " << str;
	throw std::logic_error(ss.str());
}

unsigned simple_selector::calculate_specificity()const noexcept{
	//TODO:
	return 0;
}
