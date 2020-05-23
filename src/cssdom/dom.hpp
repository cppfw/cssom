#pragma once

#include <papki/file.hpp>

#include <utki/destructable.hpp>
#include <utki/span.hpp>

#include <map>

namespace cssdom{

struct styleable{
	virtual const std::string& get_id()const = 0;
	virtual const std::string& get_tag()const = 0;

	virtual utki::span<const std::string> get_classes()const = 0;

	virtual ~styleable()noexcept{}
};

struct xml_dom_crawler{
	virtual const styleable& get() = 0;

	/**
	 * @brief Move crawler to parent node.
	 * @return true if moved.
	 * @return false if already at root node, could not move to parent node.
	 */
	virtual bool move_up() = 0;

	/**
	 * @brief Move crawler to preceding child.
	 * @return true if moved.
	 * @return false if already at the first node, could not move to the preceding node.
	 */
	virtual bool move_left() = 0;

	/**
	 * @brief Reset the crawler.
	 * This function should reset the crawler to its initial state,
	 * so that it points to the document node for which the property value is queried.
	 */
	virtual void reset() = 0;

	virtual ~xml_dom_crawler()noexcept{}
};

//TODO: doxygen all
enum class combinator{
	none,
	descendant,
	child,
	next_sibling,
	subsequent_sibling
};

/**
 * @brief Simple CSS selector.
 * The 'simple selector' term is defined in CSS spec.
 * selectors can be combined into a selector chain with combinators.
 */
struct selector{
	/**
	 * @brief Id selector.
	 * The id selector is specified with '#' in the CSS.
	 */
	std::string id;

	/**
	 * @brief Tag name.
	 * The selector tag name can also be empty or '*'.
	 */
	std::string tag;

	std::vector<std::string> classes;

	// TODO: attribute selectors, pseido-class, pseudo-element etc.

	/**
	 * @brief Combinator with next selector in the selector chain.
	 */
	cssdom::combinator combinator = cssdom::combinator::none;

	bool is_matching(const styleable& node)const;
};

/**
 * @brief List of style properties corresponding to a CSS selector.
 */
typedef std::map<uint32_t, std::unique_ptr<utki::destructable>> property_list;

/**
 * @brief Simple selector chain.
 */
typedef std::vector<selector> selector_chain;

struct style{
	selector_chain selectors;
	std::shared_ptr<property_list> properties;

	unsigned specificity;

	void update_specificity()noexcept;

	bool is_matching(xml_dom_crawler& crawler)const;
};

struct document{
	std::vector<style> styles;

	void write(
			papki::file& fi,
			const std::map<uint32_t, std::string>& property_id_to_name_map,
			const std::function<std::string(uint32_t, const utki::destructable&)>& property_value_to_string
		)const;

	void write(
			papki::file&& fi,
			const std::map<uint32_t, std::string>& property_id_to_name_map,
			const std::function<std::string(uint32_t, const utki::destructable&)>& property_value_to_string
		)const
	{
		this->write(fi, property_id_to_name_map, property_value_to_string);
	}

	void sort_styles_by_specificity();

	/**
	 * @brief Get property value for given xml document node.
	 * @return pointer to the property value if given node has matched to some CSS selector which defines requested property.
	 * @return nullptr if given node has not matched to any CSS selector or no matching selectors define requested property.
	 */
	utki::destructable* get_property_value(xml_dom_crawler& crawler, uint32_t property_id);
};

document read(
		const papki::file& fi,
		const std::map<std::string, uint32_t>& property_name_to_id_map,
		const std::function<std::unique_ptr<utki::destructable>(uint32_t, std::string&&)>& parse_property_value
	);

}