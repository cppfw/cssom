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

#include "om.hpp"

#include <algorithm>

#include <utki/string.hpp>
#include <utki/util.hpp>

#include "parser.hpp"

#ifdef assert
#	undef assert
#endif

using namespace cssom;

namespace {
combinator parse_combinator(const std::string& str)
{
	if (str.empty()) {
		return combinator::descendant;
	} else if (str == ">") {
		return combinator::child;
	} else if (str == "+") {
		return combinator::next_sibling;
	} else if (str == "~") {
		return combinator::subsequent_sibling;
	}
	std::stringstream ss;
	ss << "unknown combinator: " << str;
	throw std::logic_error(ss.str());
}
} // namespace

namespace {
std::string combinator_to_string(combinator c)
{
	switch (c) {
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
} // namespace

namespace {
class om_parser : public parser
{
	selector cur_selector;
	selector_chain cur_selector_chain;
	std::shared_ptr<property_list> cur_property_list;
	std::string cur_property_name;

public:
	sheet doc;

	std::function<uint32_t(std::string_view)> property_name_to_id;
	std::function<std::unique_ptr<cssom::property_value_base>(uint32_t, std::string_view)> parse_property;

	om_parser(
		std::function<uint32_t(std::string_view)> property_name_to_id,
		std::function<std::unique_ptr<cssom::property_value_base>(uint32_t, std::string_view)> parse_property
	) :
		property_name_to_id(std::move(property_name_to_id)),
		parse_property(std::move(parse_property))
	{}

	void on_selector_chain_end() override
	{
		// TRACE(<< "selector chain END" << std::endl)
		if (!this->cur_property_list) {
			this->cur_property_list = std::make_shared<property_list>();
		}
		// NOLINTNEXTLINE(modernize-use-designated-initializers, "need C++20 for that, while we use C++17")
		style s{
			std::move(cur_selector_chain),
			cur_property_list // several selectors may refer the same property list, therefore not moving
		};
		s.update_specificity();
		doc.styles.emplace_back(std::move(s));
		ASSERT(this->cur_selector_chain.empty())
		ASSERT(this->cur_selector.classes.empty())
		ASSERT(this->cur_selector.tag.empty())
		// TODO: add assert(attributes of current selector are empty)
	}

	void on_selector_end() override
	{
		// TRACE(<< "selector END" << std::endl)
		this->cur_selector_chain.push_back(std::move(this->cur_selector));
	}

	void on_selector_tag(std::string str) override
	{
		// TRACE(<< "selector tag: " << str << std::endl)
		this->cur_selector.tag = std::move(str);
	}

	void on_selector_id(std::string str) override
	{
		// TRACE(<< "selector id: " << str << std::endl)
		this->cur_selector.id = std::move(str);
	}

	void on_selector_class(std::string str) override
	{
		// TRACE(<< "selector class: " << str << std::endl)
		this->cur_selector.classes.push_back(std::move(str));
	}

	void on_combinator(std::string str) override
	{
		// TRACE(<< "combinator: " << str << std::endl)
		ASSERT(this->cur_selector.classes.empty())
		ASSERT(this->cur_selector.tag.empty())
		// TODO: add assert attributes of current selector are empty
		ASSERT(!this->cur_selector_chain.empty())
		this->cur_selector_chain.back().combinator = ::parse_combinator(str);
	}

	void on_style_properties_end() override
	{
		// TRACE(<< "style properties END" << std::endl)
		this->cur_property_list.reset();
	}

	void on_property_name(std::string str) override
	{
		// TRACE(<< "property name: " << str << std::endl)
		ASSERT(this->cur_property_list)
		this->cur_property_name = std::move(str);
	}

	void on_property_value(std::string str) override
	{
		// TRACE(<< "property value: " << str << std::endl)

		ASSERT(!this->cur_property_name.empty())

		ASSERT(this->property_name_to_id)
		uint32_t id = this->property_name_to_id(this->cur_property_name);

		ASSERT(this->parse_property)
		auto value = this->parse_property(id, std::move(str));

		if (!value) {
			// could not parse style property value, ignore
			return;
		}

		ASSERT(this->cur_property_list)
		(*this->cur_property_list)[id] = std::move(value);
	}
};
} // namespace

sheet cssom::read(
	const papki::file& fi,
	std::function<uint32_t(std::string_view)> property_name_to_id,
	std::function<std::unique_ptr<cssom::property_value_base>(uint32_t, std::string_view)> parse_property
)
{
	if (!property_name_to_id) {
		throw std::logic_error("cssom::read(): passed in 'property_name_to_id' function is nullptr");
	}
	if (!parse_property) {
		throw std::logic_error("cssom::read(): passed in 'parse_property' function is nullptr");
	}

	om_parser p(std::move(property_name_to_id), std::move(parse_property));

	{
		papki::file::guard file_guard(fi);

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
		std::array<uint8_t, size_t(utki::kilobyte) * 4> buf;

		while (true) {
			auto res = fi.read(utki::make_span(buf));
			utki::assert(res <= buf.size(), SL);
			if (res == 0) {
				break;
			}
			p.feed(utki::make_span(buf.data(), res));
		}
	}

	p.doc.sort_styles_by_specificity();

	return std::move(p.doc);
}

namespace {
const auto comma = utki::make_span(", ");
const auto period = utki::make_span(".");
const auto hash_sign = utki::make_span("#");
const auto open_curly_brace = utki::make_span(" {\n");
const auto tab_char = utki::make_span("\t");
const auto new_line_char = utki::make_span("\n");
const auto close_curly_brace = utki::make_span("}\n");
const auto semicolon = utki::make_span("; ");
const auto colon = utki::make_span(": ");
} // namespace

namespace {
std::string get_name(const style& st)
{
	std::stringstream ss;

	for (const auto& s : st.selectors) {
		ss << s.tag;

		if (!s.id.empty()) {
			ss << hash_sign << s.id;
		}

		for (auto& c : s.classes) {
			ss << period << c;
		}

		// TODO: selector attributes

		ss << combinator_to_string(s.combinator);
	}
	return ss.str();
}
} // namespace

namespace {
std::string to_string(
	const property_list& prop_list,
	const std::function<std::string(uint32_t)>& property_id_to_name,
	const std::function<std::string(uint32_t, const property_value_base&)>& property_value_to_string
)
{
	std::stringstream ss;
	for (auto& prop : prop_list) {
		auto name = property_id_to_name(prop.first);

		if (name.empty()) {
			continue;
		}

		ss << name << colon;
		auto value = property_value_to_string(prop.first, *prop.second);
		ss << value << semicolon;
	}

	return ss.str();
}
} // namespace

namespace {
// Produces deterministic list of CSS rules.
// The list is sorted by property groups and within a group is sorted by rule "name".
std::vector<std::pair<std::string, std::shared_ptr<std::string>>> to_string_styles(
	utki::span<const style> styles, //
	const std::function<std::string(uint32_t)>& property_id_to_name,
	const std::function<std::string(uint32_t, const property_value_base&)>& property_value_to_string
)
{
	std::map<property_list*, std::shared_ptr<std::string>> props_map;

	std::vector<std::pair<std::string, std::shared_ptr<std::string>>> str_styles;

	for (const auto& s : styles) {
		auto props_ptr = s.properties.get();
		ASSERT(props_ptr)
		auto props_iter = props_map.find(props_ptr);
		if (props_iter == props_map.end()) {
			auto res = props_map.insert(
				std::make_pair(
					props_ptr,
					std::make_shared<std::string>(to_string(
						*s.properties, //
						property_id_to_name,
						property_value_to_string
					))
				)
			);
			ASSERT(res.second)
			props_iter = res.first;
		}
		ASSERT(props_iter != props_map.end())

		str_styles.emplace_back(
			get_name(s), //
			props_iter->second
		);
	}

	std::sort(
		str_styles.begin(), //
		str_styles.end(),
		[](const auto& a, const auto& b) {
			// sort by property_list to make consequent gropus using same property list by name within group
			if (*a.second < *b.second) {
				return true;
			} else if (*a.second > *b.second) {
				return false;
			}

			// sort by name within the group
			return a.first < b.first;
		}
	);

	return str_styles;
}
} // namespace

void sheet::write(
	papki::file& fi,
	const std::function<std::string(uint32_t)>& property_id_to_name,
	const std::function<std::string(uint32_t, const property_value_base&)>& property_value_to_string,
	std::string_view indent
) const
{
	auto styles_to_save = to_string_styles(
		this->styles, //
		property_id_to_name,
		property_value_to_string
	);

	papki::file::guard file_guard(fi, papki::mode::create);

	for (auto i = styles_to_save.begin(); i != styles_to_save.end(); ++i) {
		// Go through selector chains which refer to the same property set (selectors in the same selector group).
		// These are selector chains specified as comma separated list before defining their properties in CSS sheet,
		// such selector chains will go in a row.
		auto selector_group_start_iter = i;
		for (auto j = selector_group_start_iter; j != styles_to_save.end(); ++j) {
			if (j->second.get() != selector_group_start_iter->second.get()) {
				ASSERT(j > i)
				i = --j;
				break;
			}

			i = j;

			if (j != selector_group_start_iter) {
				fi.write(comma);
			} else {
				fi.write(utki::make_span(indent));
			}

			fi.write(j->first);
		}

		// write properties
		fi.write(open_curly_brace);
		fi.write(utki::make_span(indent));
		fi.write(tab_char);

		ASSERT(selector_group_start_iter->second)

		fi.write(*selector_group_start_iter->second);

		fi.write(new_line_char);
		fi.write(utki::make_span(indent));
		fi.write(close_curly_brace);
	}
}

void sheet::sort_styles_by_specificity()
{
	std::sort(
		this->styles.begin(), //
		this->styles.end(),
		[](const auto& a, const auto& b) -> bool {
			return a.specificity > b.specificity; // descending order
		}
	);
}

void style::update_specificity() noexcept
{
	// According to CSS spec (https://www.w3.org/TR/2018/CR-selectors-3-20180130/#specificity) we need to:
	// - count the number of ID selectors in the selector (= a)
	// - count the number of class selectors, attributes selectors, and pseudo-classes in the selector (= b)
	// - count the number of type selectors and pseudo-elements in the selector (= c)
	// - ignore the universal selector
	// and then concatenate the three numbers 'abc' (in a number system with a large base) to get the overall selector
	// chain specificity.

	unsigned num_ids = 0;
	unsigned num_classes = 0;
	unsigned num_types = 0;
	for (auto& s : this->selectors) {
		if (!s.id.empty()) {
			++num_ids;
		}
		if (!s.tag.empty()) {
			if (s.tag.back() != '*') { // if not a universal selector
				++num_types;
			}
			// TODO: add pseudo-elements to num_types
		}
		num_classes += unsigned(s.classes.size());
		// TODO: add num attributes and pseudo-classes to num_classes
	}

	using std::min;

	unsigned max_val = utki::byte_mask;
	this->specificity = (uint32_t(min(max_val, num_ids)) << (utki::byte_bits * 2)) | //
		(uint32_t(min(max_val, num_classes)) << utki::byte_bits) | //
		uint32_t(min(max_val, num_types));
}

bool selector::is_matching(const styleable& node) const
{
	if (!this->tag.empty() && this->tag.back() != '*') {
		if (this->tag != node.get_tag()) {
			return false;
		}
	}

	if (!this->id.empty()) {
		if (this->id != node.get_id()) {
			return false;
		}
	}

	auto nc = node.get_classes();
	for (auto& cls : this->classes) {
		if (std::find(nc.begin(), nc.end(), cls) == nc.end()) {
			return false;
		}
	}

	// TODO: attribute selectors

	return true;
}

namespace {
bool is_descendant_matching(xml_dom_crawler& crawler, const selector& sel)
{
	while (crawler.move_up()) {
		if (sel.is_matching(crawler.get())) {
			return true;
		}
	}
	return false;
}
} // namespace

namespace {
bool is_child_matching(xml_dom_crawler& crawler, const selector& sel)
{
	if (!crawler.move_up()) {
		return false;
	}
	return sel.is_matching(crawler.get());
}
} // namespace

namespace {
bool is_next_sibling_matching(xml_dom_crawler& crawler, const selector& sel)
{
	if (!crawler.move_left()) {
		return false;
	}
	return sel.is_matching(crawler.get());
}
} // namespace

namespace {
bool is_subsequent_sibling_matching(xml_dom_crawler& crawler, const selector& sel)
{
	while (crawler.move_left()) {
		if (sel.is_matching(crawler.get())) {
			return true;
		}
	}
	return false;
}
} // namespace

bool style::is_matching(xml_dom_crawler& crawler) const
{
	for (auto i = this->selectors.rbegin(); i != this->selectors.rend(); ++i) {
		switch (i->combinator) {
			case combinator::none:
				if (!i->is_matching(crawler.get())) {
					return false;
				}
				break;
			case combinator::descendant:
				if (!is_descendant_matching(crawler, *i)) {
					return false;
				}
				break;
			case combinator::child:
				if (!is_child_matching(crawler, *i)) {
					return false;
				}
				break;
			case combinator::next_sibling:
				if (!is_next_sibling_matching(crawler, *i)) {
					return false;
				}
				break;
			case combinator::subsequent_sibling:
				if (!is_subsequent_sibling_matching(crawler, *i)) {
					return false;
				}
				break;
		}
	}

	return true;
}

sheet::query_result sheet::get_property_value(xml_dom_crawler& crawler, uint32_t property_id) const
{
	for (auto& s : this->styles) {
		crawler.reset();

		if (s.is_matching(crawler)) {
			auto i = s.properties->find(property_id);
			if (i != s.properties->end()) {
				// NOLINTNEXTLINE(modernize-use-designated-initializers, "need C++20 for that, while we use C++17")
				return query_result{i->second.get(), s.specificity};
			}
		}
	}

	// NOLINTNEXTLINE(modernize-use-designated-initializers, "need C++20 for that, while we use C++17")
	return query_result{nullptr, 0};
}

void sheet::append(sheet d)
{
	using std::begin;
	using std::end;
	std::move(begin(d.styles), end(d.styles), std::back_inserter(this->styles));
	d.styles.clear();
	this->sort_styles_by_specificity();
}
