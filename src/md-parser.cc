#include "md-parser.h"

#include <cmark-gfm-core-extensions.h>
#include <filesystem>
#include <node.h>
#include <stdexcept>
#include <string>
#include <syntax_extension.h>

static const int OPTIONS = CMARK_OPT_STRIKETHROUGH_DOUBLE_TILDE;

/// Meyers Singleton
Parser::Parser() = default;
/// Destructor
Parser::~Parser() = default;

/**
 * \brief Get singleton instance
 * \return Helper reference (singleton)
 */
Parser& Parser::getInstance()
{
  static Parser instance;
  return instance;
}

/**
 * Parse markdown file from string content.
 * Note: Do not forgot to execute: cmark_node_free(document); when you are done with the doc.
 * @return AST structure (of type cmark_node)
 */
cmark_node* Parser::parseContent(const std::string& content)
{
  const char* data = content.c_str();

  cmark_gfm_core_extensions_ensure_registered();

  // Modified version of cmark_parse_document() in blocks.c
  cmark_parser* parser = cmark_parser_new(OPTIONS);
  cmark_node* document;
  // Add extensions
  addMarkdownExtension(parser, "strikethrough");
  addMarkdownExtension(parser, "highlight");
  addMarkdownExtension(parser, "superscript");
  addMarkdownExtension(parser, "subscript");
  // addMarkdownExtension(parser, "table");

  cmark_parser_feed(parser, data, strlen(data));
  document = cmark_parser_finish(parser);
  cmark_parser_free(parser);
  return document;
}

/**
 * Built-in cmark parser to HTML
 */
std::string const Parser::renderHTML(cmark_node* node)
{
  char* tmp = cmark_render_html(node, OPTIONS, NULL);
  std::string output = std::string(tmp);
  free(tmp);
  return output;
}

/**
 * Built-in cmark parser to markdown (again)
 */
std::string const Parser::renderMarkdown(cmark_node* node)
{
  char* tmp = cmark_render_commonmark(node, OPTIONS, 600);
  std::string output = std::string(tmp);
  free(tmp);
  return output;
}

/**
 * This is a function that will make enabling extensions easier
 */
void Parser::addMarkdownExtension(cmark_parser* parser, const char* extName)
{
  cmark_syntax_extension* ext = cmark_find_syntax_extension(extName);
  if (ext)
    cmark_parser_attach_syntax_extension(parser, ext);
}
