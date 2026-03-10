#include "parser.h"

#include <cstdint>
#include <string>
#include <vector>

// ---- DOM Node ----

struct DomNode
{
  std::string tag_name;
  std::string class_attr;
  std::string text;
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
static OpenTagResult parse_opening_tag(const std::string &raw_content, int parent_index)
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

static std::vector<DomNode> build_dom(const std::string &html)
{
  std::vector<DomNode> nodes;
  // Pre-sized parent stack: all slots are within size(), so push/pop become index ops,
  // avoiding the libc++/ASan push_back-after-pop_back container-annotation issue.
  std::vector<int> open_stack((html.size() / 3) + 1, -1);
  std::size_t stack_depth = 0;

  std::size_t pos = 0;
  while (pos < html.size()) {
    if (html[pos] != '<') {
      auto end = html.find('<', pos);
      if (end == std::string::npos) { end = html.size(); }
      std::string text = html.substr(pos, end - pos);
      if (!text.empty()) {
        DomNode node;
        node.text = std::move(text);
        node.parent_index = (stack_depth == 0) ? -1 : open_stack[stack_depth - 1];
        nodes.push_back(std::move(node));
      }
      pos = end;
      continue;
    }

    auto end = html.find('>', pos);
    if (end == std::string::npos) { break; }

    if (pos + 1 < html.size() && html[pos + 1] == '/') {
      if (stack_depth > 0) { --stack_depth; }
      pos = end + 1;
      continue;
    }

    std::string tag_content = html.substr(pos + 1, end - pos - 1);
    int parent = (stack_depth == 0) ? -1 : open_stack[stack_depth - 1];
    auto [node, self_closing] = parse_opening_tag(tag_content, parent);

    int node_idx = static_cast<int>(nodes.size());
    nodes.push_back(std::move(node));
    if (!self_closing) { open_stack[stack_depth++] = node_idx; }
    pos = end + 1;
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
  if (!part.class_name.empty() && node.class_attr != part.class_name) { return false; }
  return true;
}


static std::string collect_text(const std::vector<DomNode> &nodes, int element_index)
{
  std::string result;
  for (const auto &node : nodes) {
    if (!node.is_element && node.parent_index == element_index) { result += node.text; }
  }
  return result;
}

static std::vector<std::string> match_single(const std::vector<DomNode> &nodes, const SelectorPart &part)
{
  std::vector<std::string> results;
  for (std::size_t idx = 0; idx < nodes.size(); ++idx) {
    if (matches_part(nodes[idx], part)) { results.push_back(collect_text(nodes, static_cast<int>(idx))); }
  }
  return results;
}

static std::vector<std::string>
  match_two_part(const std::vector<DomNode> &nodes, const SelectorPart &ancestor_part, const SelectorPart &target_part)
{
  std::vector<std::string> results;

  for (std::size_t idx = 0; idx < nodes.size(); ++idx) {
    if (!matches_part(nodes[idx], target_part)) { continue; }

    int node_idx = static_cast<int>(idx);
    if (target_part.combinator == Combinator::Child) {
      int parent_idx = nodes[idx].parent_index;
      if (parent_idx != -1 && matches_part(nodes[static_cast<std::size_t>(parent_idx)], ancestor_part)) {
        results.push_back(collect_text(nodes, node_idx));
      }
    } else {
      // Walk up the parent chain to find any matching ancestor
      int current = nodes[idx].parent_index;
      while (current != -1) {
        if (matches_part(nodes[static_cast<std::size_t>(current)], ancestor_part)) {
          results.push_back(collect_text(nodes, node_idx));
          break;
        }
        current = nodes[static_cast<std::size_t>(current)].parent_index;
      }
    }
  }

  return results;
}

// ---- Public API ----

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
std::vector<std::string> parse(const std::string &html, const std::string &selector)
{
  std::vector<DomNode> nodes = build_dom(html);
  std::vector<SelectorPart> parts = parse_selector(selector);

  if (parts.size() == 1) { return match_single(nodes, parts[0]); }
  if (parts.size() == 2) { return match_two_part(nodes, parts[0], parts[1]); }
  return {};
}
