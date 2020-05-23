#include "../../src/cssdom/dom.hpp"

#include <map>

#include <papki/span_file.hpp>

#include <utki/tree.hpp>

namespace{
class dom_node : public cssdom::styleable{
public:
	std::string id;
	std::string tag;

	std::vector<std::string> classes;

	dom_node(std::string&& tag, std::string&& id, std::vector<std::string>&& classes = std::vector<std::string>()) :
			id(id),
			tag(tag),
			classes(classes)
	{}

	dom_node(std::string&& tag, std::vector<std::string>&& classes = std::vector<std::string>()) :
			tag(tag),
			classes(classes)
	{}

	const std::string& get_id()const override{
		return this->id;
	}

	const std::string& get_tag()const override{
		return this->tag;
	}

	utki::span<const std::string> get_classes()const override{
		return utki::make_span(this->classes);
	}
};
}

namespace{
enum class property_id{
	fill,
	stroke,
	stroke_width
};
std::map<std::string, uint32_t> property_name_to_id_map{
	{"fill", uint32_t(property_id::fill)},
	
};
}

namespace{
struct property_value : public utki::destructable{
	std::string value;
};
}

namespace{
class crawler : public cssdom::xml_dom_crawler{
	const utki::tree<dom_node>::container_type& root;
	typedef std::remove_reference<decltype(root)>::type container_type;
	std::vector<std::pair<
			container_type*,
			container_type::const_iterator
		>> stack;
	
	const std::vector<size_t> index; // initial position
public:
	crawler(const utki::tree<dom_node>::container_type& root, std::vector<size_t>&& index) :
			root(std::move(root)),
			index(std::move(index))
	{
		this->reset();
	}

	void reset()override{
		this->stack.clear();

		if(this->index.empty()){
			this->stack.push_back(std::make_pair(&this->root, this->root.begin()));
			return;
		}
		
		auto cont = &this->root;
		auto iter = cont->begin();
		
		for(auto i : this->index){
			std::advance(iter, i);
			this->stack.push_back(std::make_pair(cont, iter));
			cont = &iter->children;
			iter = cont->begin();
		}
	}
	
	bool move_left()override{
		ASSERT(this->stack.back().first)
		auto& c = *this->stack.back().first;
		auto& i = this->stack.back().second;

		if(i == c.begin()){
			return false;
		}

		--i;
		return true;
	}

	bool move_up()override{
		if(this->stack.size() == 1){
			return false;
		}

		this->stack.pop_back();
		return true;
	}

	const cssdom::styleable& get()override{
		return this->stack.back().second->value;
	}
};
}

int main(int argc, char** argv){
	// test basic property value query
	{
		auto css = R"qwertyuiop(
			rect {
				fill: red;
				stroke: blue;
				stroke-width: 3
			}
		)qwertyuiop";

		auto css_dom = cssdom::read(
				papki::span_file(utki::make_span(css)),
				property_name_to_id_map,
				[](uint32_t id, std::string&& v) -> std::unique_ptr<utki::destructable> {
					auto ret = std::make_unique<property_value>();
					ret->value = std::move(v);
					return ret;
				}
			);

		typedef utki::tree<dom_node> node;
		node::container_type dom{
			node(dom_node("body"), {
				node(dom_node("rect"))
			})
		};

		crawler cr(dom, {0, 0});
		
		// auto p = css_dom.get_property_value(cr, uint32_t(property_id::stroke));
		// ASSERT_ALWAYS(p);

		// ASSERT_ALWAYS(dynamic_cast<property_value*>(p))

		// auto pv = static_cast<property_value*>(p);
		// ASSERT_ALWAYS(pv->value == "blue")
	}

	return 0;
}
