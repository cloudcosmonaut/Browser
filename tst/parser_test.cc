#include "md-parser.h"
#include "gtest/gtest.h"
#include <cmark-gfm.h>
#include <node.h>
namespace
{
  TEST(LibreWebTest, TestContentParser)
  {
    // Given
    uint16_t expectedDocument = CMARK_NODE_DOCUMENT;

    // When
    cmark_node* doc = Parser::parseContent("Jaja");

    // Then
    ASSERT_EQ(doc->type, expectedDocument);
    cmark_node_free(doc);
  }


  TEST(LibreWebTest, TestHTMLParser)
  {
    // Given
    std::string expectedHtml = "<p><em>Italic</em> <strong>BOLD</strong> <del>strike</del></p>\n";

    // When
    cmark_node* doc = Parser::parseContent("_Italic_ **BOLD** ~~strike~~");
    std::string html = Parser::renderHTML(doc);
    cmark_node_free(doc);

    // Then
    ASSERT_EQ(html, expectedHtml);
  }

  

} // namespace