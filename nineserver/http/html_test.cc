#include "base/testing.h"
#include "nineserver/http/html.h"

TEST(HtmlTest, EscapeHtml) {
  EXPECT_EQ("&lt;a href=&quot;foo&quot;&gt;&amp;amp; a&apos;b&lt;/a&gt;",
            EscapeHtml(R"(<a href="foo">&amp; a'b</a>)"));
  EXPECT_EQ("", EscapeHtml(""));
  EXPECT_EQ("foo", EscapeHtml("foo"));
  EXPECT_EQ("あいうえお", EscapeHtml("あいうえお"));
}

TEST(HtmlTest, UnescapeHtml) {
  EXPECT_EQ("<foo>", UnescapeHtml("&lt;foo&gt;"));
  EXPECT_EQ("<<<", UnescapeHtml("&#60;&#x3c;&lt;"));
  EXPECT_EQ("∞ ∞ ∞", UnescapeHtml("&infin; &#8734; &#x221E;"));
}
