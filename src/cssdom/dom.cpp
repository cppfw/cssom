#include "dom.hpp"
#include "parser.hpp"

using namespace cssdom;

namespace{
combinator parse_combinator(const std::string& str){
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
}

namespace{
std::string combinator_to_string(combinator c){
	switch(c){
		case combinator::descendant:
			return " ";
		case combinator::child:
			return ">";
		case combinator::next_sibling:
			return "+";
		case combinator::subsequent_sibling:
			return "~";
		default:
			return "";
	}
}
}

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
		this->cur_selector.combinator = ::parse_combinator(str);
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

namespace{
auto comma = utki::make_span(", ");
auto period = utki::make_span(".");
auto open_curly_brace = utki::make_span(" {\n\t");
auto close_curly_brace = utki::make_span("\n}\n");
auto semicolon = utki::make_span("; ");
auto colon = utki::make_span(": ");
}

void document::write(
		papki::file& fi,
		const std::map<uint32_t, std::string>& property_id_to_name_map,
		const std::function<std::string(uint32_t, const utki::destructable&)>& property_value_to_string
	)
{
	papki::file::guard file_guard(fi, papki::file::mode::create);

	for(auto i = this->styles.begin(); i != this->styles.end(); ++i){
		auto selector_group_start_iter = i;

		// go through selectors which refer to the same property set (selectors in the same selector group)
		for(auto j = selector_group_start_iter; j != this->styles.end(); ++j){
			if(j->properties.get() != selector_group_start_iter->properties.get()){
				ASSERT(j > i)
				i = --j;
				break;
			}

			if(j != selector_group_start_iter){
				fi.write(comma);
			}

			// go through selectors in the selector chain
			for(auto s = j->selectors.begin(); s != j->selectors.end(); ++s){
				if(s != j->selectors.begin()){
					// write combinator
					auto c = combinator_to_string(s->combinator);
					fi.write(utki::make_span(reinterpret_cast<const uint8_t*>(c.data()), c.size()));
				}
				// write selector tag
				fi.write(utki::make_span(s->tag));

				// write selctor classes
				for(auto& c : s->classes){
					fi.write(period);
					fi.write(utki::make_span(c));
				}

				// TODO: write attributes
			}
		}

		// write properties
		fi.write(open_curly_brace);

		auto props = selector_group_start_iter->properties.get();
		ASSERT(props)
		for(auto& prop : *props){
			auto name_iter = property_id_to_name_map.find(prop.first);
			if(name_iter == property_id_to_name_map.end()){
				continue;
			}
			fi.write(utki::make_span(name_iter->second));
			fi.write(colon);
			auto value = property_value_to_string(prop.first, *prop.second);
			fi.write(utki::make_span(value));
			fi.write(semicolon);
		}

		fi.write(close_curly_brace);
	}
}

namespace{
unsigned calc_dec_order(unsigned x){
	unsigned ret = 1;

	while(x / ret){
		ret *= 10;
	}

	return ret;
}
}

unsigned style::calculate_specificity()const noexcept{
	// According to CSS spec (https://www.w3.org/TR/2018/CR-selectors-3-20180130/#specificity) we need to:
	// - count the number of ID selectors in the selector (= a)
    // - count the number of class selectors, attributes selectors, and pseudo-classes in the selector (= b)
    // - count the number of type selectors and pseudo-elements in the selector (= c)
    // - ignore the universal selector
	// and then concatenate the thee numbers 'a:b:c' to get the overall selector chain specificity.

	unsigned num_ids = 0;
	unsigned num_classes = 0;
	unsigned num_types = 0;
	for(auto& s : this->selectors){
		if(!s.id.empty()){
			++num_ids;
		}
		if(!s.tag.empty()){
			if(s.tag.back() != '*'){ // if not a universal selector
				++num_types;
			}
			//TODO: add pseudo-elements to num_types
		}
		num_classes += s.classes.size();
		// TODO: add num attributes and pseudo-classes to num_classes
	}

	// concatenate decimal numbers 'num_ids:num_classes:num_types'

	unsigned num_classes_shift = calc_dec_order(num_types);
	unsigned num_ids_shift = calc_dec_order(num_classes) + num_classes_shift;

	return num_types + num_classes_shift * num_classes + num_ids_shift * num_ids;
}
