#include "dom.hpp"
#include "parser.hpp"

using namespace cssdom;

namespace{
class dom_parser : public parser{
	selector cur_selector;
	selector_chain cur_selector_chain;
	std::shared_ptr<property_list> cur_property_list;
	std::string cur_property_name;
public:
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

	virtual void on_selector_chain_end()override{
		TRACE(<< "selector chain END" << std::endl)
		if(!this->cur_property_list){
			this->cur_property_list = std::make_shared<property_list>();
		}
		style s{
			std::move(cur_selector_chain),
			cur_property_list // several selectors may refer the same property list, therefore not moving
		};
		doc.styles.emplace_back(std::move(s));
		ASSERT(this->cur_selector_chain.empty())
		ASSERT(this->cur_selector.classes.empty())
		ASSERT(this->cur_selector.tag.empty())
		//TODO: add assert attributes of current selector are empty
	}

	virtual void on_selector_end()override{
		TRACE(<< "selector END" << std::endl)
		this->cur_selector_chain.push_back(std::move(this->cur_selector));
	}

	virtual void on_selector_tag(std::string&& str)override{
		TRACE(<< "selector tag: " << str << std::endl)
		this->cur_selector.tag = std::move(str);
	}

	virtual void on_selector_class(std::string&& str)override{
		TRACE(<< "selector class: " << str << std::endl)
		this->cur_selector.classes.push_back(std::move(str));
	}

	virtual void on_combinator(std::string&& str)override{
		TRACE(<< "combinator: " << str << std::endl)
		ASSERT(this->cur_selector.classes.empty())
		ASSERT(this->cur_selector.tag.empty())
		//TODO: add assert attributes of current selector are empty
		this->cur_selector.combinator = cssdom::parse_combinator(str);
	}

	virtual void on_style_properties_end()override{
		TRACE(<< "style properties END" << std::endl)
		this->cur_property_list.reset();
	}

	virtual void on_property_name(std::string&& str)override{
		TRACE(<< "property name: " << str << std::endl)
		ASSERT(this->cur_property_list)
		this->cur_property_name = std::move(str);
	}

	virtual void on_property_value(std::string&& str)override{
		TRACE(<< "property value: " << str << std::endl)
		
		ASSERT(!this->cur_property_name.empty())

		auto i = this->property_name_to_id_map.find(this->cur_property_name);
		this->cur_property_name.clear();
		if(i == this->property_name_to_id_map.end()){
			// unknown property name, ignore
			return;
		}

		uint32_t id = i->second;

		ASSERT(this->parse_property)
		auto value = this->parse_property(id, std::move(str));

		if(!value){
			// could not parse style property value, ignore
			return;
		}

		ASSERT(this->cur_property_list)
		(*this->cur_property_list)[id] = std::move(value);
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

unsigned selector::calculate_specificity()const noexcept{
	//TODO:
	return 0;
}
