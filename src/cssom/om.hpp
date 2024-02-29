/*
MIT License

Copyright (c) 2020-2023 Ivan Gagis

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

#include <papki/file.hpp>
#include <utki/destructable.hpp>
#include <utki/span.hpp>

namespace cssom {

struct styleable {
	virtual std::string_view get_id() const = 0;
	virtual std::string_view get_tag() const = 0;

	virtual utki::span<const std::string> get_classes() const = 0;

	styleable() = default;

	styleable(const styleable&) = default;
	styleable& operator=(const styleable&) = default;

	styleable(styleable&&) = default;
	styleable& operator=(styleable&&) = default;

	virtual ~styleable() noexcept = default;
};

struct xml_dom_crawler {
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

// TODO: doxygen all
enum class combinator {
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

	std::vector<std::string> classes;

	// TODO: attribute selectors, pseudo-class, pseudo-element etc.

	/**
	 * @brief Combinator with next selector in the selector chain.
	 */
	cssom::combinator combinator = cssom::combinator::none;

	bool is_matching(const styleable& node) const;
};

struct property_value_base : public utki::destructable {};

/**
 * @brief List of style properties corresponding to a CSS selector.
 */
using property_list = std::map<uint32_t, std::unique_ptr<property_value_base>>;

/**
 * @brief Simple selector chain.
 */
using selector_chain = std::vector<selector>;

struct style {
	selector_chain selectors{};
	std::shared_ptr<property_list> properties{};

	unsigned specificity{};

	void update_specificity() noexcept;

	bool is_matching(xml_dom_crawler& crawler) const;
};

struct sheet {
	std::vector<style> styles{};

	void write(
		papki::file& fi,
		const std::function<std::string(uint32_t)>& property_id_to_name,
		const std::function<std::string(uint32_t, const property_value_base&)>& property_value_to_string,
		const std::string& indent = std::string()
	) const;

	void sort_styles_by_specificity();

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
		unsigned specificity;
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

sheet read(
	const papki::file& fi,
	std::function<uint32_t(std::string_view)> property_name_to_id,
	std::function<std::unique_ptr<property_value_base>(uint32_t, std::string_view)> parse_property_value
);

} // namespace cssom
