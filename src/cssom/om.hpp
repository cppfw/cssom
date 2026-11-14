/*
MIT License

Copyright (c) 2020-2024 Ivan Gagis

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

#pragma once

#include <map>

#include <fsif/file.hpp>
#include <utki/destructable.hpp>
#include <utki/span.hpp>

namespace cssom {

/**
 * @brief An element which can be styled with CSS.
 */
struct styleable {
	/**
	 * @brief Get the element id.
	 * The element's id, which can be referenced from CSS using an id selector.
	 * @return the element id.
	 */
	virtual std::string_view get_id() const = 0;

	/**
	 * @brief Get the element tag name.
	 * @return the element tag name.
	 */
	virtual std::string_view get_tag() const = 0;

	/**
	 * @brief Get the element class names.
	 * @return span of the element class names.
	 */
	virtual utki::span<const std::string> get_classes() const = 0;

	/**
	 * @brief Default constructor.
	 */
	styleable() = default;

	styleable(const styleable&) = default;
	styleable& operator=(const styleable&) = default;

	styleable(styleable&&) = default;
	styleable& operator=(styleable&&) = default;

	/**
	 * @brief Default destructor.
	 */
	virtual ~styleable() noexcept = default;
};

/**
 * @brief A crawler over an XML document.
 * Used to iterate over the XML document nodes when matching CSS selectors.
 */
struct xml_dom_crawler {
	/**
	 * @brief Get the currently crawled document node.
	 * @return the currently crawled document node.
	 */
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

	xml_dom_crawler() = default;

	xml_dom_crawler(const xml_dom_crawler&) = default;
	xml_dom_crawler& operator=(const xml_dom_crawler&) = default;

	xml_dom_crawler(xml_dom_crawler&&) = default;
	xml_dom_crawler& operator=(xml_dom_crawler&&) = default;

	virtual ~xml_dom_crawler() noexcept = default;
};

/**
 * @brief CSS selector combinator.
 * The combinator defines how a selector is related to the previous selector
 * in the selector chain.
 */
enum class combinator {
	/**
	 * @brief No combinator.
	 */
	none,
	/**
	 * @brief Descendant combinator.
	 * Selects all descendants of the previous selector.
	 */
	descendant,
	/**
	 * @brief Child combinator.
	 * Selects all direct children of the previous selector.
	 */
	child,
	/**
	 * @brief Next sibling combinator.
	 * Selects the element which is the first sibling immediately after the
	 * element of the previous selector.
	 */
	next_sibling,
	/**
	 * @brief Subsequent sibling combinator.
	 * Selects all siblings of the element of the previous selector which come
	 * after it.
	 */
	subsequent_sibling
};

/**
 * @brief Simple CSS selector.
 * The 'simple selector' term is defined in CSS spec.
 * selectors can be combined into a selector chain with combinators.
 */
// TODO: why lint complains here io macos?
// NOLINTNEXTLINE(bugprone-exception-escape, "error: an exception may be thrown")
struct selector {
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

	/**
	 * @brief Class names.
	 * The element must belong to all of the given classes to match the selector.
	 */
	std::vector<std::string> classes;

	// TODO: attribute selectors, pseudo-class, pseudo-element etc.

	/**
	 * @brief Combinator with next selector in the selector chain.
	 */
	cssom::combinator combinator = cssom::combinator::none;

	/**
	 * @brief Check if the selector matches the given document node.
	 * @param node - document node to check the selector against.
	 * @return true if the selector matches the given document node.
	 */
	bool is_matching(const styleable& node) const;
};

/**
 * @brief Base class for CSS property values.
 */
struct property_value_base : public utki::destructable {};

/**
 * @brief List of style properties corresponding to a CSS selector.
 */
using property_list = std::map<uint32_t, std::unique_ptr<property_value_base>>;

/**
 * @brief Simple selector chain.
 */
using selector_chain = std::vector<selector>;

/**
 * @brief A CSS style, i.e. a single entry in a CSS style sheet.
 * A style consists of a selector chain and a list of style properties corresponding to it.
 */
struct style {
	/**
	 * @brief Selector chain.
	 */
	selector_chain selectors{};

	/**
	 * @brief List of style properties corresponding to the selector chain.
	 */
	std::shared_ptr<property_list> properties{};

	/**
	 * @brief Selector specificity.
	 */
	uint32_t specificity{};

	/**
	 * @brief Recalculate the selector specificity.
	 */
	void update_specificity() noexcept;

	/**
	 * @brief Check if the style matches the currently crawled document node.
	 * @param crawler - document node crawler.
	 * @return true if the style matches the currently crawled document node.
	 */
	bool is_matching(xml_dom_crawler& crawler) const;
};

/**
 * @brief A CSS style sheet, i.e. a list of CSS styles.
 */
struct sheet {
	/**
	 * @brief Styles of the style sheet.
	 */
	std::vector<style> styles{};

	/**
	 * @brief Write the style sheet to a file.
	 * @param fi - file to write the style sheet to. File must not be opened.
	 * @param property_id_to_name - function to convert a property id to a property name.
	 * @param property_value_to_string - function to convert a property value to a string.
	 * @param indent - indentation string to use for formatting.
	 */
	void write(
		fsif::file& fi,
		const std::function<std::string(uint32_t)>& property_id_to_name,
		const std::function<std::string(uint32_t, const property_value_base&)>& property_value_to_string,
		std::string_view indent = {}
	) const;

	/**
	 * @brief Sort the styles of the style sheet by specificity.
	 */
	void sort_styles_by_specificity();

	/**
	 * @brief Append styles from another style sheet.
	 * @param d - style sheet to append the styles from.
	 */
	void append(sheet d);

	struct query_result {
		/**
		 * @brief Value of the queried property.
		 * Can be nullptr if no property was found.
		 */
		const property_value_base* value;

		/**
		 * @brief Selector specificity.
		 * Speceficity of the selector which matched the element for which the property value was queried.
		 */
		uint32_t specificity;
	};

	/**
	 * @brief Get property value for given xml document node.
	 * @return pointer to the property value if given node has matched to some CSS selector which defines requested
	 * property.
	 * @return nullptr if given node has not matched to any CSS selector or no matching selectors define requested
	 * property.
	 */
	query_result get_property_value(xml_dom_crawler& crawler, uint32_t property_id) const;
};

/**
 * @brief Read a CSS style sheet from a file.
 * @param fi - file to read the style sheet from. File must not be opened.
 * @param property_name_to_id - function to convert a property name to a property id.
 * @param parse_property_value - function to parse a property value string into a property value object.
 * @return the read style sheet.
 */
sheet read(
	const fsif::file& fi,
	std::function<uint32_t(std::string_view)> property_name_to_id,
	std::function<std::unique_ptr<property_value_base>(uint32_t, std::string_view)> parse_property_value
);

} // namespace cssom
