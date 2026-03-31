#include "parser.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// ---- DOM Node ----

struct DomNode
{
  std::size_t open_start = 0;// byte offset of '<' in original html
  std::size_t close_end = 0;// byte offset after '>' of closing tag (or self-closing '>')
  std::string tag_name;
  std::string class_attr;
  std::string text;// only populated for text nodes (!is_element)
  int parent_index = -1;
  bool is_element = false;
};

// ---- Selector Part ----

enum class Combinator : std::uint8_t { None, Descendant, Child };

struct SelectorPart
{
  std::string tag_name;
  std::string class_name;
  Combinator combinator = Combinator::None;
};

// ---- HTML Tokenizer / DOM Builder ----

static std::size_t skip_whitespace(const std::string &str, std::size_t pos)
{
  while (pos < str.size() && (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\n' || str[pos] == '\r')) { ++pos; }
  return pos;
}

static std::string extract_class(const std::string &attr_str)
{
  const std::string key = "class=";
  auto pos = attr_str.find(key);
  if (pos == std::string::npos) { return ""; }

  pos += key.size();
  if (pos >= attr_str.size()) { return ""; }

  char quote = attr_str[pos];
  if (quote != '"' && quote != '\'') { return ""; }

  ++pos;
  auto end_pos = attr_str.find(quote, pos);
  if (end_pos == std::string::npos) { return ""; }

  return attr_str.substr(pos, end_pos - pos);
}

struct OpenTagResult
{
  DomNode node;
  bool self_closing{};
};

// Pure function: parses tag content string and returns a DomNode + self-closing flag.
// Does not touch any shared state, safe across ASAN container boundaries.
// string_view passed by value -- idiomatic, avoids indirection for a trivially-copyable type.
static OpenTagResult parse_opening_tag(std::string_view raw_content, int parent_index)
{
  const bool self_closing = !raw_content.empty() && raw_content.back() == '/';
  const std::string_view content(raw_content.data(), raw_content.size() - (self_closing ? 1 : 0));

  auto space_pos = content.find_first_of(" \t\n\r");
  DomNode node;
  node.tag_name =
    (space_pos == std::string_view::npos) ? std::string(content) : std::string(content.substr(0, space_pos));
  node.class_attr =
    extract_class((space_pos == std::string_view::npos) ? std::string{} : std::string(content.substr(space_pos)));
  node.is_element = true;
  node.parent_index = parent_index;

  return { .node = std::move(node), .self_closing = self_closing };
}

// ---- DOM Builder Helpers ----
// Each handles one branch of the tokenizer loop, keeping build_dom's complexity low.

static std::size_t process_text_node(const std::string &html,
  std::size_t pos,
  const std::vector<int> &open_stack,
  std::vector<DomNode> &nodes)
{
  auto end = html.find('<', pos);
  if (end == std::string::npos) { end = html.size(); }
  std::string text = html.substr(pos, end - pos);
  if (!text.empty()) {
    DomNode node;
    node.text = std::move(text);
    node.parent_index = open_stack.empty() ? -1 : open_stack.back();
    nodes.push_back(std::move(node));
  }
  return end;
}

// Records close_end on the node being closed, then pops it from the stack.
static std::size_t process_closing_tag(std::size_t end, std::vector<int> &open_stack, std::vector<DomNode> &nodes)
{
  if (!open_stack.empty()) {
    // close_end is byte after '>' so substr(open_start, close_end - open_start) spans the full element.
    nodes[static_cast<std::size_t>(open_stack.back())].close_end = end + 1;
    open_stack.pop_back();
  }
  return end + 1;
}

static std::size_t process_opening_tag(const std::string &html,
  std::size_t pos,
  std::size_t end,
  std::vector<int> &open_stack,
  std::vector<DomNode> &nodes)
{
  std::string_view tag_content = std::string_view(html).substr(pos + 1, end - pos - 1);
  int parent = open_stack.empty() ? -1 : open_stack.back();
  auto [node, self_closing] = parse_opening_tag(tag_content, parent);
  node.open_start = pos;
  if (self_closing) { node.close_end = end + 1; }
  int node_idx = static_cast<int>(nodes.size());
  nodes.push_back(std::move(node));
  if (!self_closing) { open_stack.push_back(node_idx); }
  return end + 1;
}

// Builds a flat DOM tree: a vector of nodes each knowing their parent index and byte offsets in html.
static std::vector<DomNode> build_dom(const std::string &html)
{
  std::vector<DomNode> nodes;
  std::vector<int> open_stack;
  // Conservative upper bound — reserve to avoid repeated heap re-allocation.
  open_stack.reserve((html.size() / 3) + 1);
  nodes.reserve((html.size() / 3) + 1);

  std::size_t pos = 0;
  while (pos < html.size()) {
    if (html[pos] != '<') {
      pos = process_text_node(html, pos, open_stack, nodes);
      continue;
    }
    auto end = html.find('>', pos);
    if (end == std::string::npos) { break; }// malformed HTML
    if (pos + 1 < html.size() && html[pos + 1] == '/') {
      pos = process_closing_tag(end, open_stack, nodes);
    } else {
      pos = process_opening_tag(html, pos, end, open_stack, nodes);
    }
  }

  return nodes;
}

// ---- Selector Parser ----

static SelectorPart parse_simple_selector(const std::string &selector_str)
{
  SelectorPart part;
  auto dot_pos = selector_str.find('.');
  if (dot_pos == 0) {
    part.class_name = selector_str.substr(1);
  } else if (dot_pos != std::string::npos) {
    part.tag_name = selector_str.substr(0, dot_pos);
    part.class_name = selector_str.substr(dot_pos + 1);
  } else {
    part.tag_name = selector_str;
  }
  return part;
}

static std::vector<SelectorPart> parse_selector(const std::string &selector)
{
  std::vector<SelectorPart> parts;

  auto child_pos = selector.find(" > ");
  if (child_pos != std::string::npos) {
    parts.push_back(parse_simple_selector(selector.substr(0, child_pos)));
    SelectorPart right = parse_simple_selector(selector.substr(child_pos + 3));
    right.combinator = Combinator::Child;
    parts.push_back(right);
    return parts;
  }

  auto space_pos = selector.find(' ');
  if (space_pos != std::string::npos) {
    parts.push_back(parse_simple_selector(selector.substr(0, space_pos)));
    SelectorPart right = parse_simple_selector(selector.substr(skip_whitespace(selector, space_pos)));
    right.combinator = Combinator::Descendant;
    parts.push_back(right);
    return parts;
  }

  parts.push_back(parse_simple_selector(selector));
  return parts;
}

// ---- Matching ----

static bool matches_part(const DomNode &node, const SelectorPart &part)
{
  if (!node.is_element) { return false; }
  if (!part.tag_name.empty() && node.tag_name != part.tag_name) { return false; }
  // Class attribute may contain multiple space-separated class names;
  // check that part.class_name appears as a whole token, not just a substring.
  if (!part.class_name.empty()) {
    std::size_t pos = 0;
    while ((pos = node.class_attr.find(part.class_name, pos)) != std::string::npos) {
      bool at_start = (pos == 0 || node.class_attr[pos - 1] == ' ');
      bool at_end = (pos + part.class_name.size() == node.class_attr.size()
                     || node.class_attr[pos + part.class_name.size()] == ' ');
      if (at_start && at_end) { return true; }
      ++pos;
    }
    return false;
  }
  return true;
}

// Reconstructs the full HTML of a matched element via a single substr on the original html.
// Works for nested content because open_start..close_end spans the entire element verbatim.
static std::string reconstruct_element(const std::string &html, const std::vector<DomNode> &nodes, int element_index)
{
  const auto &node = nodes[static_cast<std::size_t>(element_index)];
  return html.substr(node.open_start, node.close_end - node.open_start);
}

static std::vector<std::string>
  match_selector(const std::string &html, const std::vector<DomNode> &nodes, const SelectorPart &part)
{
  std::vector<std::string> results;
  for (std::size_t idx = 0; idx < nodes.size(); ++idx) {
    if (matches_part(nodes[idx], part)) { results.push_back(reconstruct_element(html, nodes, static_cast<int>(idx))); }
  }
  return results;
}

static std::vector<std::string> match_compound_selector(const std::string &html,
  const std::vector<DomNode> &nodes,
  const SelectorPart &ancestor_part,
  const SelectorPart &target_part)
{
  std::vector<std::string> results;

  for (std::size_t idx = 0; idx < nodes.size(); ++idx) {
    if (!matches_part(nodes[idx], target_part)) { continue; }

    int node_idx = static_cast<int>(idx);
    if (target_part.combinator == Combinator::Child) {
      int parent_idx = nodes[idx].parent_index;
      if (parent_idx != -1 && matches_part(nodes[static_cast<std::size_t>(parent_idx)], ancestor_part)) {
        results.push_back(reconstruct_element(html, nodes, node_idx));
      }
    } else {
      // Walk up the parent chain to find any matching ancestor
      int current = nodes[idx].parent_index;
      while (current != -1) {
        if (matches_part(nodes[static_cast<std::size_t>(current)], ancestor_part)) {
          results.push_back(reconstruct_element(html, nodes, node_idx));
          break;
        }
        current = nodes[static_cast<std::size_t>(current)].parent_index;
      }
    }
  }

  return results;
}

// ---- Public API ----
// Entry point into the code, the "main" function equiv. Takes in the raw HTML and a CSS selector.

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
std::vector<std::string> parse(const std::string &html, const std::string &selector)
{
  std::vector<DomNode> nodes = build_dom(html);
  std::vector<SelectorPart> parts = parse_selector(selector);

  if (parts.size() == 1) { return match_selector(html, nodes, parts[0]); }
  if (parts.size() == 2) { return match_compound_selector(html, nodes, parts[0], parts[1]); }
  return {};
}

// ---- Text Extraction Utilities ----

// Removes all <script> and <style> blocks (including their content) from html.
static std::string strip_inert_blocks(const std::string &html)
{
  std::string result = html;
  for (const std::string &tag : { std::string("script"), std::string("style") }) {
    std::string open = "<" + tag;
    std::string close = "</" + tag + ">";
    std::size_t start = 0;
    while ((start = result.find(open, start)) != std::string::npos) {
      auto end = result.find(close, start);
      if (end == std::string::npos) { break; }
      result.erase(start, end + close.size() - start);
    }
  }
  return result;
}

static std::string normalize_whitespace(const std::string &str)
{
  std::string out;
  out.reserve(str.size());
  bool in_space = true;// starts true to trim leading whitespace
  for (char ch : str) {
    if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
      if (!in_space) {
        out += ' ';
        in_space = true;
      }
    } else {
      out += ch;
      in_space = false;
    }
  }
  if (!out.empty() && out.back() == ' ') { out.pop_back(); }
  return out;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
std::string extract_href(const std::string &html, const std::string &base_url)
{
  const std::string marker = "href=\"";
  auto href_start = html.find(marker);
  if (href_start == std::string::npos) { return {}; }
  href_start += marker.size();

  auto href_end = html.find('"', href_start);
  if (href_end == std::string::npos) { return {}; }

  std::string href = html.substr(href_start, href_end - href_start);

  constexpr std::string_view http = "http://";
  constexpr std::string_view https = "https://";
  if (href.substr(0, http.size()) == http || href.substr(0, https.size()) == https) { return href; }

  return base_url + href;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
std::string extract_inner_text(const std::string &html, const std::string &tag)
{
  const std::string cleaned = strip_inert_blocks(html);
  std::string result;
  const std::string open_tag = "<" + tag;
  const std::string closing_tag = "</" + tag + ">";

  std::size_t pos = 0;
  while (pos < cleaned.size()) {
    auto tag_start = cleaned.find(open_tag, pos);
    if (tag_start == std::string::npos) { break; }

    auto content_start = cleaned.find('>', tag_start);
    if (content_start == std::string::npos) { break; }
    ++content_start;

    auto content_end = cleaned.find(closing_tag, content_start);
    if (content_end == std::string::npos) { break; }

    if (!result.empty()) { result += ' '; }

    // Strip nested HTML tags by skipping '<' ... '>' sequences.
    for (std::size_t i = content_start; i < content_end; ++i) {
      if (cleaned[i] == '<') {
        auto close = cleaned.find('>', i);
        if (close == std::string::npos || close >= content_end) { break; }
        i = close;
      } else {
        result += cleaned[i];
      }
    }

    pos = content_end + closing_tag.size();
  }

  return normalize_whitespace(result);
}
