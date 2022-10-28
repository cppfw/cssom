/*
MIT License

Copyright (c) 2020-2022 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/* ================ LICENSE END ================ */

#include "om.hpp"
#include "parser.hpp"

using namespace cssom;

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
			return " > ";
		case combinator::next_sibling:
			return " + ";
		case combinator::subsequent_sibling:
			return " ~ ";
		case combinator::none:
		default:
			return "";
	}
}
}

namespace{
class om_parser : public parser{
	selector cur_selector;
	selector_chain cur_selector_chain;
	std::shared_ptr<property_list> cur_property_list;
	std::string cur_property_name;
public:
	sheet doc;

	const std::function<uint32_t(const std::string&)>& property_name_to_id;
	const std::function<std::unique_ptr<cssom::property_value_base>(uint32_t, std::string&&)>& parse_property;

	om_parser(
			const std::function<uint32_t(const std::string&)>& property_name_to_id,
			const std::function<std::unique_ptr<cssom::property_value_base>(uint32_t, std::string&&)>& parse_property
		) :
			property_name_to_id(property_name_to_id),
			parse_property(parse_property)
	{}

	virtual void on_selector_chain_end()override{
		// TRACE(<< "selector chain END" << std::endl)
		if(!this->cur_property_list){
			this->cur_property_list = std::make_shared<property_list>();
		}
		style s{
			std::move(cur_selector_chain),
			cur_property_list // several selectors may refer the same property list, therefore not moving
		};
		s.update_specificity();
		doc.styles.emplace_back(std::move(s));
		ASSERT(this->cur_selector_chain.empty())
		ASSERT(this->cur_selector.classes.empty())
		ASSERT(this->cur_selector.tag.empty())
		//TODO: add assert attributes of current selector are empty
	}

	virtual void on_selector_end()override{
		// TRACE(<< "selector END" << std::endl)
		this->cur_selector_chain.push_back(std::move(this->cur_selector));
	}

	virtual void on_selector_tag(std::string&& str)override{
		// TRACE(<< "selector tag: " << str << std::endl)
		this->cur_selector.tag = std::move(str);
	}

	virtual void on_selector_class(std::string&& str)override{
		// TRACE(<< "selector class: " << str << std::endl)
		this->cur_selector.classes.push_back(std::move(str));
	}

	virtual void on_combinator(std::string&& str)override{
		// TRACE(<< "combinator: " << str << std::endl)
		ASSERT(this->cur_selector.classes.empty())
		ASSERT(this->cur_selector.tag.empty())
		//TODO: add assert attributes of current selector are empty
		ASSERT(!this->cur_selector_chain.empty())
		this->cur_selector_chain.back().combinator = ::parse_combinator(str);
	}

	virtual void on_style_properties_end()override{
		// TRACE(<< "style properties END" << std::endl)
		this->cur_property_list.reset();
	}

	virtual void on_property_name(std::string&& str)override{
		// TRACE(<< "property name: " << str << std::endl)
		ASSERT(this->cur_property_list)
		this->cur_property_name = std::move(str);
	}

	virtual void on_property_value(std::string&& str)override{
		// TRACE(<< "property value: " << str << std::endl)
		
		ASSERT(!this->cur_property_name.empty())

		ASSERT(this->property_name_to_id)
		uint32_t id = this->property_name_to_id(this->cur_property_name);

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

sheet cssom::read(
		const papki::file& fi,
		const std::function<uint32_t(const std::string&)> property_name_to_id,
		const std::function<std::unique_ptr<cssom::property_value_base>(uint32_t, std::string&&)>& parse_property
	)
{
	if(!property_name_to_id){
		throw std::logic_error("cssom::read(): passed in 'property_name_to_id' function is nullptr");
	}
	if(!parse_property){
		throw std::logic_error("cssom::read(): passed in 'parse_property' function is nullptr");
	}

	om_parser p(property_name_to_id, parse_property);
	
	{
		papki::file::guard file_guard(fi);

		std::array<uint8_t, 4096> buf; // 4k

		while(true){
			auto res = fi.read(utki::make_span(buf));
			utki::assert(res <= buf.size(), SL);
			if(res == 0){
				break;
			}
			p.feed(utki::make_span(buf.data(), res));
		}
	}
	
	p.doc.sort_styles_by_specificity();

	return std::move(p.doc);
}

namespace{
auto comma = utki::make_span(", ");
auto period = utki::make_span(".");
auto open_curly_brace = utki::make_span(" {\n");
auto tab_char = utki::make_span("\t");
auto new_line_char = utki::make_span("\n");
auto close_curly_brace = utki::make_span("}\n");
auto semicolon = utki::make_span("; ");
auto colon = utki::make_span(": ");
}

void sheet::write(
		papki::file& fi,
		const std::function<std::string(uint32_t)>& property_id_to_name,
		const std::function<std::string(uint32_t, const property_value_base&)>& property_value_to_string,
		const std::string& indent
	)const
{
	papki::file::guard file_guard(fi, papki::file::mode::create);

	for(auto i = this->styles.begin(); i != this->styles.end(); ++i){
		// Go through selector chains which refer to the same property set (selectors in the same selector group).
		// These are selector chains specified as comma separated list before defining their properties in CSS sheet,
		// such selector chains will go in a row.
		auto selector_group_start_iter = i;
		for(auto j = selector_group_start_iter; j != this->styles.end(); ++j){
			if(j->properties.get() != selector_group_start_iter->properties.get()){
				ASSERT(j > i)
				i = --j;
				break;
			}

			i = j;

			if(j != selector_group_start_iter){
				fi.write(comma);
			}else{
				fi.write(utki::make_span(indent));
			}

			// go through selectors in the selector chain
			for(auto s = j->selectors.begin(); s != j->selectors.end(); ++s){
				// write selector tag
				fi.write(utki::make_span(s->tag));

				// write selctor classes
				for(auto& c : s->classes){
					fi.write(period);
					fi.write(utki::make_span(c));
				}

				// TODO: write attributes

				// write combinator
				auto c = combinator_to_string(s->combinator);
				fi.write(utki::make_span(c));
			}
		}

		// write properties
		fi.write(open_curly_brace);
		fi.write(utki::make_span(indent));
		fi.write(tab_char);

		auto props = selector_group_start_iter->properties.get();
		ASSERT(props)
		for(auto& prop : *props){
			auto name = property_id_to_name(prop.first);
			
			if(name.empty()){
				continue;
			}

			fi.write(utki::make_span(name));
			fi.write(colon);
			auto value = property_value_to_string(prop.first, *prop.second);
			fi.write(utki::make_span(value));
			fi.write(semicolon);
		}

		fi.write(new_line_char);
		fi.write(utki::make_span(indent));
		fi.write(close_curly_brace);
	}
}

void sheet::sort_styles_by_specificity(){
	std::sort(
			this->styles.begin(),
			this->styles.end(),
			[](
					const decltype(this->styles)::value_type& a,
					const decltype(this->styles)::value_type& b
				) -> bool
			{
				return a.specificity > b.specificity; // descending order
			}
		);
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

void style::update_specificity()noexcept{
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
		num_classes += unsigned(s.classes.size());
		// TODO: add num attributes and pseudo-classes to num_classes
	}

	// concatenate decimal numbers 'num_ids:num_classes:num_types'

	unsigned num_classes_shift = calc_dec_order(num_types);
	unsigned num_ids_shift = calc_dec_order(num_classes) + num_classes_shift;

	this->specificity = num_types + num_classes_shift * num_classes + num_ids_shift * num_ids;
}

bool selector::is_matching(const styleable& node)const{
	if(!this->tag.empty() && this->tag.back() != '*'){
		if(this->tag != node.get_tag()){
			return false;
		}
	}

	if(!this->id.empty()){
		if(this->id != node.get_id()){
			return false;
		}
	}

	auto nc = node.get_classes();
	for(auto& cls : this->classes){
		if(std::find(nc.begin(), nc.end(), cls) == nc.end()){
			return false;
		}
	}

	// TODO: attribute selectors

	return true;
}

namespace{
bool is_descendant_matching(xml_dom_crawler& crawler, const selector& sel){
	while(crawler.move_up()){
		if(sel.is_matching(crawler.get())){
			return true;
		}
	}
	return false;
}
}

namespace{
bool is_child_matching(xml_dom_crawler& crawler, const selector& sel){
	if(!crawler.move_up()){
		return false;
	}
	return sel.is_matching(crawler.get());
}
}

namespace{
bool is_next_sibling_matching(xml_dom_crawler& crawler, const selector& sel){
	if(!crawler.move_left()){
		return false;
	}
	return sel.is_matching(crawler.get());
}
}

namespace{
bool is_subsequent_sibling_matching(xml_dom_crawler& crawler, const selector& sel){
	while(crawler.move_left()){
		if(sel.is_matching(crawler.get())){
			return true;
		}
	}
	return false;
}
}

bool style::is_matching(xml_dom_crawler& crawler)const{
	for(auto i = this->selectors.rbegin(); i != this->selectors.rend(); ++i){
		switch(i->combinator){
			case combinator::none:
				if(!i->is_matching(crawler.get())){
					return false;
				}
				break;
			case combinator::descendant:
				if(!is_descendant_matching(crawler, *i)){
					return false;
				}
				break;
			case combinator::child:
				if(!is_child_matching(crawler, *i)){
					return false;
				}
				break;
			case combinator::next_sibling:
				if(!is_next_sibling_matching(crawler, *i)){
					return false;
				}
				break;
			case combinator::subsequent_sibling:
				if(!is_subsequent_sibling_matching(crawler, *i)){
					return false;
				}
				break;
		}
	}

	return true;
}

sheet::query_result sheet::get_property_value(xml_dom_crawler& crawler, uint32_t property_id)const{
	for(auto& s : this->styles){
		crawler.reset();
		
		if(s.is_matching(crawler)){
			auto i = s.properties->find(property_id);
			if(i != s.properties->end()){
				return query_result{i->second.get(), s.specificity};
			}
		}
	}
	
	return query_result{nullptr, 0};
}

void sheet::append(sheet&& d){
	using std::begin;
	using std::end;
	std::move(begin(d.styles), end(d.styles), std::back_inserter(this->styles));
	d.styles.clear();
	this->sort_styles_by_specificity();
}
