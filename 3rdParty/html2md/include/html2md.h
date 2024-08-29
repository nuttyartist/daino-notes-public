// Copyright (c) Tim Gromeyer
// Licensed under the MIT License - https://opensource.org/licenses/MIT

#ifndef HTML2MD_H
#define HTML2MD_H

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/*!
 * \brief html2md namespace
 *
 * The html2md namespace provides:
 * 1. The Converter class
 * 2. Static wrapper around Converter class
 *
 * \note Do NOT try to convert HTML that contains a list in an ordered list or a
 * `blockquote` in a list!\n  This will be a **total** mess!
 */
namespace html2md {

/*!
 * \brief Options for the conversion from HTML to Markdown
 * \warning Make sure to pass valid options; otherwise, the output will be
 * invalid!
 *
 * Example from `tests/main.cpp`:
 *
 * ```cpp
 * auto *options = new html2md::Options();
 * options->splitLines = false;
 *
 * html2md::Converter c(html, options);
 * auto md = c.convert();
 * ```
 */
struct Options {
  /*!
   * \brief Add new line when a certain number of characters is reached
   */
  bool splitLines = true;
  bool trim = true;

  /*!
   * \brief The char used for unordered lists
   *
   * Valid:
   * - `-`
   * - `+`
   * - `*`
   *
   * Example:
   *
   * ```markdown
   * - List
   * + Also a list
   * * And this to
   * ```
   */
  char unorderedList = '-';

  /*!
   * \brief The char used after the number of the item
   *
   * Valid:
   * - `.`
   * - `)`
   *
   * Example:
   *
   * ```markdown
   * 1. Hello
   * 2) World!
   * ```
   */
  char orderedList = '.';

  /*!
   * \brief Whether title is added as h1 heading at the very beginning of the
   * markdown
   *
   * Whether title is added as h1 heading at the very beginning of the markdown.
   * Default is true.
   */
  bool includeTitle = true;

  /*!
   * \brief Whetever to format Markdown Tables
   *
   * Whetever to format Markdown Tables.
   * Default is true.
   */
  bool formatTable = true;

  inline bool operator==(html2md::Options o) const {
    return splitLines == o.splitLines && unorderedList == o.unorderedList &&
           orderedList == o.orderedList && includeTitle == o.includeTitle;
  };
};

/*!
 * \brief Class for converting HTML to Markdown
 *
 * This class converts HTML to Markdown.
 * There is also a static wrapper for this class (see html2md::Convert).
 *
 * ## Usage example
 *
 * Option 1: Use the class:
 *
 * ```cpp
 * std::string html = "<h1>example</h1>";
 * html2md::Converter c(html);
 * auto md = c.convert();
 *
 * if (!c.ok()) std::cout << "There was something wrong in the HTML\n";
 * std::cout << md; // # example
 * ```
 *
 * Option 2: Use the static wrapper:
 *
 * ```cpp
 * std::string html = "<h1>example</h1>";
 *
 * auto md = html2md::Convert(html);
 * std::cout << md;
 * ```
 *
 * Advanced: use Options:
 *
 * ```cpp
 * std::string html = "<h1>example</h1>";
 *
 * auto *options = new html2md::Options();
 * options->splitLines = false;
 * options->unorderedList = '*';
 *
 * html2md::Converter c(html, options);
 * auto md = c.convert();
 * if (!c.ok()) std::cout << "There was something wrong in the HTML\n";
 * std::cout << md; // # example
 * ```
 */
class Converter {
public:
  /*!
   * \brief Standard initializer, takes HTML as parameter. Also prepares
   * everything. \param html The HTML as std::string. \param options Options for
   * the Conversation. See html2md::Options() for more.
   *
   * \note Don't pass anything else than HTML, otherwise the output will be a
   * **mess**!
   *
   * This is the default initializer.<br>
   * You can use appendToMd() to append something to the beginning of the
   * generated output.
   */
  explicit inline Converter(std::string &html,
                            struct Options *options = nullptr) {
    *this = Converter(&html, options);
  }

  /*!
   * \brief Convert HTML into Markdown.
   * \return Returns the converted Markdown.
   *
   * This function actually converts the HTML into Markdown.
   * It also cleans up the Markdown so you don't have to do anything.
   */
  [[nodiscard]] std::string convert();

  /*!
   * \brief Append a char to the Markdown.
   * \param ch The char to append.
   * \return Returns a copy of the instance with the char appended.
   */
  Converter *appendToMd(char ch);

  /*!
   * \brief Append a char* to the Markdown.
   * \param str The char* to append.
   * \return Returns a copy of the instance with the char* appended.
   */
  Converter *appendToMd(const char *str);

  /*!
   * \brief Append a string to the Markdown.
   * \param s The string to append.
   * \return Returns a copy of the instance with the string appended.
   */
  inline Converter *appendToMd(const std::string &s) {
    return appendToMd(s.c_str());
  }

  /*!
   * \brief Appends a ' ' in certain cases.
   * \return Copy of the instance with(maybe) the appended space.
   *
   * This function appends ' ' if:
   * - md does not end with `*`
   * - md does not end with `\n` aka newline
   */
  Converter *appendBlank();

  /*!
   * \brief Checks if everything was closed properly(in the HTML).
   * \return Returns false if there is a unclosed tag.
   * \note As long as you have not called convert(), it always returns true.
   */
  [[nodiscard]] bool ok() const;

  /*!
   * \brief Reset the generated Markdown
   */
  void reset();

  /*!
   * \brief Checks if the HTML matches and the options are the same.
   * \param The Converter object to compare with
   * \return true if the HTML and options matches otherwise false
   */
  inline bool operator==(const Converter *c) const { return *this == *c; }

  inline bool operator==(const Converter &c) const {
    return html_ == c.html_ && option == c.option;
  }

  /*!
   * \brief Returns ok().
   */
  inline explicit operator bool() const { return ok(); };

private:
  // Attributes
  static constexpr const char *kAttributeHref = "href";
  static constexpr const char *kAttributeAlt = "alt";
  static constexpr const char *kAttributeTitle = "title";
  static constexpr const char *kAttributeClass = "class";
  static constexpr const char *kAttributeSrc = "src";
  static constexpr const char *kAttrinuteAlign = "align";

  static constexpr const char *kTagAnchor = "a";
  static constexpr const char *kTagBreak = "br";
  static constexpr const char *kTagCode = "code";
  static constexpr const char *kTagDiv = "div";
  static constexpr const char *kTagHead = "head";
  static constexpr const char *kTagLink = "link";
  static constexpr const char *kTagListItem = "li";
  static constexpr const char *kTagMeta = "meta";
  static constexpr const char *kTagNav = "nav";
  static constexpr const char *kTagNoScript = "noscript";
  static constexpr const char *kTagOption = "option";
  static constexpr const char *kTagOrderedList = "ol";
  static constexpr const char *kTagParagraph = "p";
  static constexpr const char *kTagPre = "pre";
  static constexpr const char *kTagScript = "script";
  static constexpr const char *kTagSpan = "span";
  static constexpr const char *kTagStyle = "style";
  static constexpr const char *kTagTemplate = "template";
  static constexpr const char *kTagTitle = "title";
  static constexpr const char *kTagUnorderedList = "ul";
  static constexpr const char *kTagImg = "img";
  static constexpr const char *kTagSeperator = "hr";

  // Text format
  static constexpr const char *kTagBold = "b";
  static constexpr const char *kTagStrong = "strong";
  static constexpr const char *kTagItalic = "em";
  static constexpr const char *kTagItalic2 = "i";
  static constexpr const char *kTagCitation = "cite";
  static constexpr const char *kTagDefinition = "dfn";
  static constexpr const char *kTagUnderline = "u";
  static constexpr const char *kTagStrighthrought = "del";
  static constexpr const char *kTagStrighthrought2 = "s";

  static constexpr const char *kTagBlockquote = "blockquote";

  // Header
  static constexpr const char *kTagHeader1 = "h1";
  static constexpr const char *kTagHeader2 = "h2";
  static constexpr const char *kTagHeader3 = "h3";
  static constexpr const char *kTagHeader4 = "h4";
  static constexpr const char *kTagHeader5 = "h5";
  static constexpr const char *kTagHeader6 = "h6";

  // Table
  static constexpr const char *kTagTable = "table";
  static constexpr const char *kTagTableRow = "tr";
  static constexpr const char *kTagTableHeader = "th";
  static constexpr const char *kTagTableData = "td";

  size_t index_ch_in_html_ = 0;

  bool is_closing_tag_ = false;
  bool is_in_attribute_value_ = false;
  bool is_in_code_ = false;
  bool is_in_list_ = false;
  bool is_in_p_ = false;
  bool is_in_pre_ = false;
  bool is_in_table_ = false;
  bool is_in_table_row_ = false;
  bool is_in_tag_ = false;

  // relevant for <li> only, false = is in unordered list
  bool is_in_ordered_list_ = false;
  uint8_t index_ol = 0;

  // store the table start
  size_t table_start = 0;

  // number of lists
  uint8_t index_li = 0;

  uint8_t index_blockquote = 0;

  char prev_ch_in_md_ = 0, prev_prev_ch_in_md_ = 0;
  char prev_ch_in_html_ = 'x';

  std::string html_;

  uint16_t offset_lt_ = 0;
  std::string current_tag_;
  std::string prev_tag_;

  // Line which separates header from data
  std::string tableLine;

  size_t chars_in_curr_line_ = 0;

  std::string md_;

  Options option;

  // Tag: base class for tag types
  struct Tag {
    virtual void OnHasLeftOpeningTag(Converter *c) = 0;
    virtual void OnHasLeftClosingTag(Converter *c) = 0;
  };

  // Tag types

  // tags that are not printed (nav, script, noscript, ...)
  struct TagIgnored : Tag {
    void OnHasLeftOpeningTag(Converter *c) override{};
    void OnHasLeftClosingTag(Converter *c) override{};
  };

  struct TagAnchor : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;

    std::string current_href_;
    std::string current_title_;
  };

  struct TagBold : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagItalic : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagUnderline : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagStrikethrought : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagBreak : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagDiv : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagHeader1 : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagHeader2 : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagHeader3 : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagHeader4 : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagHeader5 : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagHeader6 : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagListItem : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagOption : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagOrderedList : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagParagraph : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagPre : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagCode : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagSpan : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagTitle : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagUnorderedList : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagImage : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagSeperator : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagTable : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagTableRow : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagTableHeader : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagTableData : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  struct TagBlockquote : Tag {
    void OnHasLeftOpeningTag(Converter *c) override;
    void OnHasLeftClosingTag(Converter *c) override;
  };

  std::unordered_map<std::string, std::shared_ptr<Tag>> tags_;

  explicit Converter(std::string *html, struct Options *options);

  void CleanUpMarkdown();

  // Trim from start (in place)
  static void LTrim(std::string *s);

  // Trim from end (in place)
  Converter *RTrim(std::string *s, bool trim_only_blank = false);

  // Trim from both ends (in place)
  Converter *Trim(std::string *s);

  // 1. trim all lines
  // 2. reduce consecutive newlines to maximum 3
  void TidyAllLines(std::string *str);

  std::string ExtractAttributeFromTagLeftOf(const std::string &attr);

  void TurnLineIntoHeader1();

  void TurnLineIntoHeader2();

  // Current char: '<'
  void OnHasEnteredTag();

  Converter *UpdatePrevChFromMd();

  /**
   * Handle next char within <...> tag
   *
   * @param ch current character
   * @return   continue surrounding iteration?
   */
  bool ParseCharInTag(char ch);

  // Current char: '>'
  bool OnHasLeftTag();

  inline static bool TagContainsAttributesToHide(std::string *tag) {
    using std::string;

    return (*tag).find(" aria=\"hidden\"") != string::npos ||
           (*tag).find("display:none") != string::npos ||
           (*tag).find("visibility:hidden") != string::npos ||
           (*tag).find("opacity:0") != string::npos ||
           (*tag).find("Details-content--hidden-not-important") != string::npos;
  }

  Converter *ShortenMarkdown(size_t chars = 1);
  inline bool shortIfPrevCh(char prev) {
    if (prev_ch_in_md_ == prev) {
      ShortenMarkdown();
      return true;
    }
    return false;
  };

  /**
   * @param ch
   * @return continue iteration surrounding  this method's invocation?
   */
  bool ParseCharInTagContent(char ch);

  // Replace previous space (if any) in current markdown line by newline
  bool ReplacePreviousSpaceInLineByNewline();

  static inline bool IsIgnoredTag(const std::string &tag) {
    return (tag[0] == '-' || kTagTemplate == tag || kTagStyle == tag ||
            kTagScript == tag || kTagNoScript == tag || kTagNav == tag);

    // meta: not ignored to tolerate if closing is omitted
  }

  [[nodiscard]] bool IsInIgnoredTag() const;
}; // Converter

/*!
 * \brief Static wrapper around the Converter class
 * \param html The HTML passed to Converter
 * \param ok Optional: Pass a reference to a local bool to store the output of
 * Converter::ok() \return Returns the by Converter generated Markdown
 */
inline std::string Convert(std::string &html, bool *ok = nullptr) {
  Converter c(html);
  auto md = c.convert();
  if (ok != nullptr)
    *ok = c.ok();
  return md;
}

#ifndef PYTHON_BINDINGS
inline std::string Convert(std::string &&html, bool *ok = nullptr) {
  return Convert(html, ok);
}
#endif

} // namespace html2md

#endif // HTML2MD_H
