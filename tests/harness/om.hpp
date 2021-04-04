#pragma once

#include <utki/tree.hpp>

#include "../../src/cssom/om.hpp"

class om_node : public cssom::styleable{
public:
	std::string id;
	std::string tag;

	std::vector<std::string> classes;

	om_node(std::string&& tag, std::string&& id, std::vector<std::string>&& classes = std::vector<std::string>()) :
			id(id),
			tag(tag),
			classes(classes)
	{}

	om_node(std::string&& tag, std::vector<std::string>&& classes = std::vector<std::string>()) :
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

class crawler : public cssom::xml_dom_crawler{
	const utki::tree<om_node>::container_type& root;
	typedef std::remove_reference<decltype(root)>::type container_type;
	std::vector<std::pair<
			container_type*,
			container_type::const_iterator
		>> stack;
	
	const std::vector<size_t> index; // initial position
public:
	crawler(const utki::tree<om_node>::container_type& root, std::vector<size_t>&& index) :
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

	const cssom::styleable& get()override{
		return this->stack.back().second->value;
	}
};

cssom::document read_css(const char* str);
