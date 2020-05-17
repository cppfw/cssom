#pragma once

#include <papki/file.hpp>

#include <utki/destructable.hpp>
#include <utki/span.hpp>

#include <map>

namespace cssdom{

//TODO: doxygen all
enum class combinator{
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
	 * @brief Combinator with previous selector in the selector chain.
	 */
	cssdom::combinator combinator = cssdom::combinator::descendant;
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
};

struct styleable{
	virtual std::string& get_id() = 0;
	virtual std::string& get_tag() = 0;

	virtual utki::span<const std::string> get_classes() = 0;

	virtual ~styleable()noexcept{}
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
};

document read(
		const papki::file& fi,
		const std::map<std::string, uint32_t>& property_name_to_id_map,
		const std::function<std::unique_ptr<utki::destructable>(uint32_t, std::string&&)>& parse_property_value
	);

}
