#pragma once

#include <papki/file.hpp>
#include <utki/destructable.hpp>

#include <map>

namespace cssdom{

//TODO: doxygen all
enum class combinator{
	descendant,
	child,
	next_sibling,
	subsequent_sibling
};

combinator parse_combinator(const std::string& str);

struct selector{
	/**
	 * @brief Tag name.
	 * The selector tag name can also be empty or *.
	 */
	std::string tag;

	std::vector<std::string> classes;

	// TODO: attribute selectors, pseido-class, pseudo-element etc.

	/**
	 * @brief Combinator with previous selector in the selector sequence.
	 */
	cssdom::combinator combinator = cssdom::combinator::descendant;

	unsigned calculate_specificity()const noexcept;
};

typedef std::map<uint32_t, std::unique_ptr<utki::destructable>> property_list;
typedef std::vector<selector> selector_sequence;

struct style{
	selector_sequence selectors;
	std::shared_ptr<property_list> properties;
};

struct document{
	std::vector<style> styles;
};

document read(
		const papki::file& fi,
		const std::map<std::string, uint32_t>& property_name_to_id_map,
		const std::function<std::unique_ptr<utki::destructable>(uint32_t, std::string&&)>& parse_property
	);

}
