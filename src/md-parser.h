#ifndef MD_PARSER_H
#define MD_PARSER_H

#include <cmark-gfm.h>
#include <render.h>
#include <sstream>
#include <string>

/**
 * \class Parser
 * \brief Parser Markdown parser class, parse the content to an AST model
 */
class Parser
{
public:
    // Singleton
    static Parser& getInstance();
    static cmark_node* parseContent(const std::string& content);
    static std::string const renderHTML(cmark_node* node);
    static std::string const renderMarkdown(cmark_node* node);

private:
    Parser();
    ~Parser();
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    static void addMarkdownExtension(cmark_parser* parser, const char* extName);
};
#endif
