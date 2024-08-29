// Copyright (c) Tim Gromeyer
// Licensed under the MIT License - https://opensource.org/licenses/MIT

#include "html2md.h"
#include "table.h"

#include <algorithm>
#include <cstring>
#include <functional>
#include <memory>
#include <sstream>
#include <stack>
#include <utility>

using std::make_shared;
using std::string;
using std::vector;

namespace {
bool startsWith(const string &str, const string &prefix) {
  return str.size() >= prefix.size() &&
         0 == str.compare(0, prefix.size(), prefix);
}

bool endsWith(const string &str, const string &suffix) {
  return str.size() >= suffix.size() &&
         0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

size_t ReplaceAll(string *haystack, const string &needle,
                  const string &replacement) {
  // Get first occurrence
  size_t pos = (*haystack).find(needle);

  size_t amount_replaced = 0;

  // Repeat until end is reached
  while (pos != string::npos) {
    // Replace this occurrence of sub string
    (*haystack).replace(pos, needle.size(), replacement);

    // Get the next occurrence from the current position
    pos = (*haystack).find(needle, pos + replacement.size());

    ++amount_replaced;
  }

  return amount_replaced;
}

size_t ReplaceAll(string *haystack, const string &needle, const char c) {
  return ReplaceAll(haystack, needle, string({c}));
}

// Split given string by given character delimiter into vector of strings
vector<string> Split(string const &str, char delimiter) {
  vector<string> result;
  std::stringstream iss(str);

  for (string token; getline(iss, token, delimiter);)
    result.push_back(token);

  return result;
}

string Repeat(const string &str, size_t amount) {
  if (amount == 0)
    return "";
  else if (amount == 1)
    return str;

  string out;

  for (size_t i = 0; i < amount; ++i)
    out.append(str);

  return out;
}
} // namespace

namespace html2md {

Converter::Converter(string *html, Options *options) : html_(*html) {
  if (options)
    option = *options;

  tags_.reserve(41);

  // non-printing tags
  auto tagIgnored = make_shared<Converter::TagIgnored>();
  tags_[kTagHead] = tagIgnored;
  tags_[kTagMeta] = tagIgnored;
  tags_[kTagNav] = tagIgnored;
  tags_[kTagNoScript] = tagIgnored;
  tags_[kTagScript] = tagIgnored;
  tags_[kTagStyle] = tagIgnored;
  tags_[kTagTemplate] = tagIgnored;

  // printing tags
  tags_[kTagAnchor] = make_shared<Converter::TagAnchor>();
  tags_[kTagBreak] = make_shared<Converter::TagBreak>();
  tags_[kTagDiv] = make_shared<Converter::TagDiv>();
  tags_[kTagHeader1] = make_shared<Converter::TagHeader1>();
  tags_[kTagHeader2] = make_shared<Converter::TagHeader2>();
  tags_[kTagHeader3] = make_shared<Converter::TagHeader3>();
  tags_[kTagHeader4] = make_shared<Converter::TagHeader4>();
  tags_[kTagHeader5] = make_shared<Converter::TagHeader5>();
  tags_[kTagHeader6] = make_shared<Converter::TagHeader6>();
  tags_[kTagListItem] = make_shared<Converter::TagListItem>();
  tags_[kTagOption] = make_shared<Converter::TagOption>();
  tags_[kTagOrderedList] = make_shared<Converter::TagOrderedList>();
  tags_[kTagPre] = make_shared<Converter::TagPre>();
  tags_[kTagCode] = make_shared<Converter::TagCode>();
  tags_[kTagParagraph] = make_shared<Converter::TagParagraph>();
  tags_[kTagSpan] = make_shared<Converter::TagSpan>();
  tags_[kTagUnorderedList] = make_shared<Converter::TagUnorderedList>();
  tags_[kTagTitle] = make_shared<Converter::TagTitle>();
  tags_[kTagImg] = make_shared<Converter::TagImage>();
  tags_[kTagSeperator] = make_shared<Converter::TagSeperator>();

  // Text formatting
  auto tagBold = make_shared<Converter::TagBold>();
  tags_[kTagBold] = tagBold;
  tags_[kTagStrong] = tagBold;

  auto tagItalic = make_shared<Converter::TagItalic>();
  tags_[kTagItalic] = tagItalic;
  tags_[kTagItalic2] = tagItalic;
  tags_[kTagDefinition] = tagItalic;
  tags_[kTagCitation] = tagItalic;

  tags_[kTagUnderline] = make_shared<Converter::TagUnderline>();

  auto tagStrighthrought = make_shared<Converter::TagStrikethrought>();
  tags_[kTagStrighthrought] = tagStrighthrought;
  tags_[kTagStrighthrought2] = tagStrighthrought;

  tags_[kTagBlockquote] = make_shared<Converter::TagBlockquote>();

  // Tables
  tags_[kTagTable] = make_shared<Converter::TagTable>();
  tags_[kTagTableRow] = make_shared<Converter::TagTableRow>();
  tags_[kTagTableHeader] = make_shared<Converter::TagTableHeader>();
  tags_[kTagTableData] = make_shared<Converter::TagTableData>();
}

void Converter::CleanUpMarkdown() {
  TidyAllLines(&md_);

  ReplaceAll(&md_, " , ", ", ");

  ReplaceAll(&md_, "\n.\n", ".\n");
  ReplaceAll(&md_, "\n↵\n", " ↵\n");
  ReplaceAll(&md_, "\n*\n", "\n");
  ReplaceAll(&md_, "\n. ", ".\n");

  ReplaceAll(&md_, "&quot;", '"');
  ReplaceAll(&md_, "&lt;", "<");
  ReplaceAll(&md_, "&gt;", ">");
  ReplaceAll(&html_, "&amp;", '&');
  ReplaceAll(&html_, "&nbsp;", ' ');
  ReplaceAll(&html_, "&rarr;", "→");

  ReplaceAll(&md_, "\t\t  ", "\t\t");
}

Converter *Converter::appendToMd(char ch) {
  if (IsInIgnoredTag())
    return this;

  if (index_blockquote != 0 && ch == '\n') {
    if (is_in_pre_) {
      md_ += ch;
      chars_in_curr_line_ = 0;
      appendToMd(Repeat("> ", index_blockquote));
    }

    return this;
  }

  md_ += ch;

  if (ch == '\n')
    chars_in_curr_line_ = 0;
  else
    ++chars_in_curr_line_;

  return this;
}

Converter *Converter::appendToMd(const char *str)
{
  if (IsInIgnoredTag())
    return this;

  md_ += str;

  auto str_len = strlen(str);

  for (auto i = 0; i < str_len; ++i) {
    if (str[i] == '\n')
      chars_in_curr_line_ = 0;
    else
      ++chars_in_curr_line_;
  }

  return this;
}

Converter *Converter::appendBlank() {
  UpdatePrevChFromMd();

  if (prev_ch_in_md_ == '\n' ||
      (prev_ch_in_md_ == '*' && prev_prev_ch_in_md_ == '*'))
    return this;

  return appendToMd(' ');
}

bool Converter::ok() const {
  return !is_in_pre_ && !is_in_list_ && !is_in_p_ && !is_in_table_ &&
         !is_in_tag_ && index_blockquote == 0 && index_li == 0;
}

void Converter::LTrim(string *s) {
  (*s).erase((*s).begin(),
             find_if((*s).begin(), (*s).end(),
                     [](unsigned char ch) { return !std::isspace(ch); }));
}

Converter *Converter::RTrim(string *s, bool trim_only_blank) {
  (*s).erase(find_if((*s).rbegin(), (*s).rend(),
                     [trim_only_blank](unsigned char ch) {
                       if (trim_only_blank)
                         return !isblank(ch);

                       return !isspace(ch);
                     })
                 .base(),
             (*s).end());

  return this;
}

// NOTE: Pay attention when changing one of the trim functions. It can break the
// output!
Converter *Converter::Trim(string *s) {
  if (!option.trim)
    return this;

  if (!startsWith(*s, "\t"))
    LTrim(s);

  if (!(startsWith(*s, "  "), endsWith(*s, "  ")))
    RTrim(s);

  return this;
}

void Converter::TidyAllLines(string *str) {
  auto lines = Split(*str, '\n');
  string res;

  uint8_t amount_newlines = 0;
  bool in_code_block = false;

  for (auto line : lines) {
    if (startsWith(line, "```") || startsWith(line, "~~~"))
      in_code_block = !in_code_block;
    if (in_code_block) {
      res += line + '\n';
      continue;
    }

    Trim(&line);

    if (line.empty()) {
      if (amount_newlines < 2 && !res.empty()) {
        res += '\n';
        amount_newlines++;
      }
    } else {
      amount_newlines = 0;

      res += line + '\n';
    }
  }

  *str = res;
}

string Converter::ExtractAttributeFromTagLeftOf(const string &attr) {
  // Extract the whole tag from current offset, e.g. from '>', backwards
  auto tag = html_.substr(offset_lt_, index_ch_in_html_ - offset_lt_);

  // locate given attribute
  auto offset_attr = tag.find(attr);

  if (offset_attr == string::npos)
    return "";

  // locate attribute-value pair's '='
  auto offset_equals = tag.find('=', offset_attr);

  if (offset_equals == string::npos)
    return "";

  // locate value's surrounding quotes
  auto offset_double_quote = tag.find('"', offset_equals);
  auto offset_single_quote = tag.find('\'', offset_equals);

  bool has_double_quote = offset_double_quote != string::npos;
  bool has_single_quote = offset_single_quote != string::npos;

  if (!has_double_quote && !has_single_quote)
    return "";

  char wrapping_quote = 0;

  size_t offset_opening_quote = 0;
  size_t offset_closing_quote = 0;

  if (has_double_quote) {
    if (!has_single_quote) {
      wrapping_quote = '"';
      offset_opening_quote = offset_double_quote;
    } else {
      if (offset_double_quote < offset_single_quote) {
        wrapping_quote = '"';
        offset_opening_quote = offset_double_quote;
      } else {
        wrapping_quote = '\'';
        offset_opening_quote = offset_single_quote;
      }
    }
  } else {
    // has only single quote
    wrapping_quote = '\'';
    offset_opening_quote = offset_single_quote;
  }

  if (offset_opening_quote == string::npos)
    return "";

  offset_closing_quote = tag.find(wrapping_quote, offset_opening_quote + 1);

  if (offset_closing_quote == string::npos)
    return "";

  return tag.substr(offset_opening_quote + 1,
                    offset_closing_quote - 1 - offset_opening_quote);
}

void Converter::TurnLineIntoHeader1() {
  appendToMd('\n' + Repeat("=", chars_in_curr_line_) + "\n\n");

  chars_in_curr_line_ = 0;
}

void Converter::TurnLineIntoHeader2() {
  appendToMd('\n' + Repeat("-", chars_in_curr_line_) + "\n\n");

  chars_in_curr_line_ = 0;
}

string Converter::convert() {
  // We already converted
  if (index_ch_in_html_ == html_.size())
    return md_;

  reset();

  for (char ch : html_) {
    ++index_ch_in_html_;

    if (!is_in_tag_ && ch == '<') {
      OnHasEnteredTag();

      continue;
    }

    if (is_in_tag_)
      ParseCharInTag(ch);
    else
      ParseCharInTagContent(ch);
  }

  CleanUpMarkdown();

  return md_;
}

void Converter::OnHasEnteredTag() {
  offset_lt_ = index_ch_in_html_;
  is_in_tag_ = true;
  prev_tag_ = current_tag_;
  current_tag_ = "";

  if (!md_.empty()) {
    UpdatePrevChFromMd();
  }
}

Converter *Converter::UpdatePrevChFromMd() {
  if (!md_.empty()) {
    prev_ch_in_md_ = md_[md_.length() - 1];

    if (md_.length() > 1)
      prev_prev_ch_in_md_ = md_[md_.length() - 2];
  }

  return this;
}

bool Converter::ParseCharInTag(char ch) {
  if (ch == '/' && current_tag_.empty()) {
    is_closing_tag_ = true;

    return true;
  }

  if (ch == '>')
    return OnHasLeftTag();

  if (ch == '=')
    return true;

  if (ch == '"') {
    if (is_in_attribute_value_) {
      is_in_attribute_value_ = false;
    } else if (prev_ch_in_md_ == '=') {
      is_in_attribute_value_ = true;
    }

    return true;
  }

  current_tag_ += ch;

  return false;
}

bool Converter::OnHasLeftTag() {
  is_in_tag_ = false;

  UpdatePrevChFromMd();

  if (!is_closing_tag_)
    if (TagContainsAttributesToHide(&current_tag_))
      return true;

  current_tag_ = Split(current_tag_, ' ')[0];

  auto tag = tags_[current_tag_];

  if (!tag)
    return true;

  if (is_closing_tag_) {
    is_closing_tag_ = false;

    tag->OnHasLeftClosingTag(this);
  } else {
    tag->OnHasLeftOpeningTag(this);
  }

  return true;
}

Converter *Converter::ShortenMarkdown(size_t chars) {
  md_ = md_.substr(0, md_.length() - chars);

  if (chars > chars_in_curr_line_)
    chars_in_curr_line_ = 0;
  else
    chars_in_curr_line_ = chars_in_curr_line_ - chars;

  return this->UpdatePrevChFromMd();
}

bool Converter::ParseCharInTagContent(char ch) {
  if (is_in_code_) {
    md_ += ch;

    if (index_blockquote != 0 && ch == '\n')
      appendToMd(Repeat("> ", index_blockquote));

    return true;
  }

  if (IsInIgnoredTag() || current_tag_ == kTagLink) {
    prev_ch_in_html_ = ch;

    return true;
  }

  if (ch == '\n') {
    if (index_blockquote != 0) {
      md_ += '\n';
      chars_in_curr_line_ = 0;
      appendToMd(Repeat("> ", index_blockquote));
    }

    return true;
  }

  switch (ch) {
  case '*':
    appendToMd("\\*");
    break;
  case '`':
    appendToMd("\\`");
    break;
  case '\\':
    appendToMd("\\\\");
    break;
  default:
    md_ += ch;
    ++chars_in_curr_line_;
    break;
  }

  if (chars_in_curr_line_ > 80 && !is_in_table_ && !is_in_list_ &&
      current_tag_ != kTagImg && current_tag_ != kTagAnchor &&
      option.splitLines) {
    if (ch == ' ') { // If the next char is - it will become a list
      md_ += '\n';
      chars_in_curr_line_ = 0;
    } else if (chars_in_curr_line_ > 100) {
      ReplacePreviousSpaceInLineByNewline();
    }
  }

  return false;
}

bool Converter::ReplacePreviousSpaceInLineByNewline() {
  if (current_tag_ == kTagParagraph ||
      is_in_table_ && (prev_tag_ != kTagCode && prev_tag_ != kTagPre))
    return false;

  auto offset = md_.length() - 1;

  if (md_.length() == 0)
    return true;

  do {
    if (md_[offset] == '\n')
      return false;

    if (md_[offset] == ' ') {
      md_[offset] = '\n';
      chars_in_curr_line_ = md_.length() - offset;

      return true;
    }

    --offset;
  } while (offset > 0);

  return false;
}

void Converter::TagAnchor::OnHasLeftOpeningTag(Converter *c) {
  if (c->prev_tag_ == kTagImg)
    c->appendToMd('\n');

  current_title_ = c->ExtractAttributeFromTagLeftOf(kAttributeTitle);

  c->appendToMd('[');
  current_href_ = c->ExtractAttributeFromTagLeftOf(kAttributeHref);
}

void Converter::TagAnchor::OnHasLeftClosingTag(Converter *c) {
  if (!c->shortIfPrevCh('[')) {
    c->appendToMd("](")->appendToMd(current_href_);

    // If title is set append it
    if (!current_title_.empty()) {
      c->appendToMd(" \"")->appendToMd(current_title_)->appendToMd('"');
      current_title_.clear();
    }

    c->appendToMd(')');

    if (c->prev_tag_ == kTagImg)
      c->appendToMd('\n');
  }
}

void Converter::TagBold::OnHasLeftOpeningTag(Converter *c) {
  c->appendToMd("**");
}

void Converter::TagBold::OnHasLeftClosingTag(Converter *c) {
  c->appendToMd("**");
}

void Converter::TagItalic::OnHasLeftOpeningTag(Converter *c) {
  c->appendToMd('*');
}

void Converter::TagItalic::OnHasLeftClosingTag(Converter *c) {
  c->appendToMd('*');
}

void Converter::TagUnderline::OnHasLeftOpeningTag(Converter *c) {
  c->appendToMd("_");
}

void Converter::TagUnderline::OnHasLeftClosingTag(Converter *c) {
  c->appendToMd("_");
}

void Converter::TagStrikethrought::OnHasLeftOpeningTag(Converter *c) {
  c->appendToMd('~');
}

void Converter::TagStrikethrought::OnHasLeftClosingTag(Converter *c) {
  c->appendToMd('~');
}

void Converter::TagBreak::OnHasLeftOpeningTag(Converter *c) {
  if (c->is_in_list_) { // When it's in a list, it's not in a paragraph
    c->appendToMd("  \n");
    c->appendToMd(Repeat("  ", c->index_li));
  } else if (c->is_in_table_) {
    c->appendToMd("<br>");
  } else if (!c->is_in_p_) {
    c->appendToMd("\n<br>\n\n");
  } else if (c->md_.length() > 0) {
    c->appendToMd("<br />");
    // c->appendToMd("  \n");
  }
}

void Converter::TagBreak::OnHasLeftClosingTag(Converter *c) {}

void Converter::TagDiv::OnHasLeftOpeningTag(Converter *c) {
  if (c->prev_ch_in_md_ != '\n')
    c->appendToMd('\n');

  if (c->prev_prev_ch_in_md_ != '\n')
    c->appendToMd('\n');
}

void Converter::TagDiv::OnHasLeftClosingTag(Converter *c) {}

void Converter::TagHeader1::OnHasLeftOpeningTag(Converter *c) {
  c->appendToMd("\n# ");
}

void Converter::TagHeader1::OnHasLeftClosingTag(Converter *c) {
  if (c->prev_prev_ch_in_md_ != ' ')
    c->appendToMd('\n');
}

void Converter::TagHeader2::OnHasLeftOpeningTag(Converter *c) {
  c->appendToMd("\n## ");
}

void Converter::TagHeader2::OnHasLeftClosingTag(Converter *c) {
  if (c->prev_prev_ch_in_md_ != ' ')
    c->appendToMd('\n');
}

void Converter::TagHeader3::OnHasLeftOpeningTag(Converter *c) {
  c->appendToMd("\n### ");
}

void Converter::TagHeader3::OnHasLeftClosingTag(Converter *c) {
  if (c->prev_prev_ch_in_md_ != ' ')
    c->appendToMd('\n');
}

void Converter::TagHeader4::OnHasLeftOpeningTag(Converter *c) {
  c->appendToMd("\n#### ");
}

void Converter::TagHeader4::OnHasLeftClosingTag(Converter *c) {
  if (c->prev_prev_ch_in_md_ != ' ')
    c->appendToMd('\n');
}

void Converter::TagHeader5::OnHasLeftOpeningTag(Converter *c) {
  c->appendToMd("\n##### ");
}

void Converter::TagHeader5::OnHasLeftClosingTag(Converter *c) {
  if (c->prev_prev_ch_in_md_ != ' ')
    c->appendToMd('\n');
}

void Converter::TagHeader6::OnHasLeftOpeningTag(Converter *c) {
  c->appendToMd("\n###### ");
}

void Converter::TagHeader6::OnHasLeftClosingTag(Converter *c) {
  if (c->prev_prev_ch_in_md_ != ' ')
    c->appendToMd('\n');
}

void Converter::TagListItem::OnHasLeftOpeningTag(Converter *c) {
  if (c->is_in_table_)
    return;

  if (!c->is_in_ordered_list_) {
    c->appendToMd(string({c->option.unorderedList, ' '}));
    return;
  }

  ++c->index_ol;

  string num = std::to_string(c->index_ol);
  num.append({c->option.orderedList, ' '});
  c->appendToMd(num);
}

void Converter::TagListItem::OnHasLeftClosingTag(Converter *c) {
  if (c->is_in_table_)
    return;

  if (c->prev_ch_in_md_ != '\n')
    c->appendToMd('\n');
}

void Converter::TagOption::OnHasLeftOpeningTag(Converter *c) {}

void Converter::TagOption::OnHasLeftClosingTag(Converter *c) {
  if (c->md_.length() > 0)
    c->appendToMd("  \n");
}

void Converter::TagOrderedList::OnHasLeftOpeningTag(Converter *c) {
  if (c->is_in_table_)
    return;

  c->is_in_list_ = true;
  c->is_in_ordered_list_ = true;
  c->index_ol = 0;

  ++c->index_li;

  c->ReplacePreviousSpaceInLineByNewline();

  c->appendToMd('\n');
}

void Converter::TagOrderedList::OnHasLeftClosingTag(Converter *c) {
  if (c->is_in_table_)
    return;

  c->is_in_ordered_list_ = false;

  if (c->index_li != 0)
    --c->index_li;

  c->is_in_list_ = c->index_li != 0;

  c->appendToMd('\n');
}

void Converter::TagParagraph::OnHasLeftOpeningTag(Converter *c) {
  c->is_in_p_ = true;

  if (c->is_in_list_ && c->prev_tag_ == kTagParagraph)
    c->appendToMd("\n\t");
  else if (!c->is_in_list_)
    c->appendToMd('\n');
}

void Converter::TagParagraph::OnHasLeftClosingTag(Converter *c) {
  c->is_in_p_ = false;

  if (!c->md_.empty())
    c->appendToMd("\n"); // Workaround \n restriction for blockquotes

  if (c->index_blockquote != 0)
    c->appendToMd(Repeat("> ", c->index_blockquote));
}

void Converter::TagPre::OnHasLeftOpeningTag(Converter *c) {
  c->is_in_pre_ = true;

  if (c->prev_ch_in_md_ != '\n')
    c->appendToMd('\n');

  if (c->prev_prev_ch_in_md_ != '\n')
    c->appendToMd('\n');

  if (c->is_in_list_ && c->prev_tag_ != kTagParagraph)
    c->ShortenMarkdown(2);

  if (c->is_in_list_)
    c->appendToMd("\t\t");
  else
    c->appendToMd("```");
}

void Converter::TagPre::OnHasLeftClosingTag(Converter *c) {
  c->is_in_pre_ = false;

  if (c->is_in_list_)
    return;

  c->appendToMd("```");
  c->appendToMd('\n'); // Don't combine because of blockquote
}

void Converter::TagCode::OnHasLeftOpeningTag(Converter *c) {
  c->is_in_code_ = true;

  if (c->is_in_pre_) {
    if (c->is_in_list_)
      return;

    auto code = c->ExtractAttributeFromTagLeftOf(kAttributeClass);
    if (!code.empty()) {
      if (startsWith(code, "language-"))
        code.erase(0, 9); // remove language-
      c->appendToMd(code);
    }
    c->appendToMd('\n');
  } else
    c->appendToMd('`');
}

void Converter::TagCode::OnHasLeftClosingTag(Converter *c) {
  c->is_in_code_ = false;

  if (c->is_in_pre_)
    return;

  c->appendToMd('`');
}

void Converter::TagSpan::OnHasLeftOpeningTag(Converter *c) {}

void Converter::TagSpan::OnHasLeftClosingTag(Converter *c) {}

void Converter::TagTitle::OnHasLeftOpeningTag(Converter *c) {}

void Converter::TagTitle::OnHasLeftClosingTag(Converter *c) {
  c->TurnLineIntoHeader1();
}

void Converter::TagUnorderedList::OnHasLeftOpeningTag(Converter *c) {
  if (c->is_in_list_ || c->is_in_table_)
    return;

  c->is_in_list_ = true;

  ++c->index_li;

  c->appendToMd('\n');
}

void Converter::TagUnorderedList::OnHasLeftClosingTag(Converter *c) {
  if (c->is_in_table_)
    return;

  if (c->index_li != 0)
    --c->index_li;

  c->is_in_list_ = c->index_li != 0;

  if (c->prev_prev_ch_in_md_ == '\n' && c->prev_ch_in_md_ == '\n')
    c->ShortenMarkdown();
  else if (c->prev_ch_in_md_ != '\n')
    c->appendToMd('\n');
}

void Converter::TagImage::OnHasLeftOpeningTag(Converter *c) {
  if (c->prev_tag_ != kTagAnchor && c->prev_ch_in_md_ != '\n')
    c->appendToMd('\n');

  c->appendToMd("![")
      ->appendToMd(c->ExtractAttributeFromTagLeftOf(kAttributeAlt))
      ->appendToMd("](")
      ->appendToMd(c->ExtractAttributeFromTagLeftOf(kAttributeSrc));

  auto title = c->ExtractAttributeFromTagLeftOf(kAttributeTitle);
  if (!title.empty()) {
    c->appendToMd(" \"")->appendToMd(title)->appendToMd('"');
  }

  c->appendToMd(")");
}

void Converter::TagImage::OnHasLeftClosingTag(Converter *c) {
  if (c->prev_tag_ == kTagAnchor)
    c->appendToMd('\n');
}

void Converter::TagSeperator::OnHasLeftOpeningTag(Converter *c) {
  c->appendToMd("\n---\n"); // NOTE: We can make this an option
}

void Converter::TagSeperator::OnHasLeftClosingTag(Converter *c) {}

void Converter::TagTable::OnHasLeftOpeningTag(Converter *c) {
  c->is_in_table_ = true;
  c->appendToMd('\n');
  c->table_start = c->md_.length();
}

void Converter::TagTable::OnHasLeftClosingTag(Converter *c) {
  c->is_in_table_ = false;
  c->appendToMd('\n');

  if (!c->option.formatTable)
    return;

  string table = c->md_.substr(c->table_start);
  table = formatMarkdownTable(table);
  c->ShortenMarkdown(c->md_.size() - c->table_start);
  c->appendToMd(table);
}

void Converter::TagTableRow::OnHasLeftOpeningTag(Converter *c) {
  c->appendToMd('\n');
}

void Converter::TagTableRow::OnHasLeftClosingTag(Converter *c) {
  c->UpdatePrevChFromMd();
  if (c->prev_ch_in_md_ == '|')
    c->appendToMd('\n'); // There's a bug
  else
    c->appendToMd('|');

  if (!c->tableLine.empty()) {
    if (c->prev_ch_in_md_ != '\n')
      c->appendToMd('\n');

    c->tableLine.append("|\n");
    c->appendToMd(c->tableLine);
    c->tableLine.clear();
  }
}

void Converter::TagTableHeader::OnHasLeftOpeningTag(Converter *c) {
  auto align = c->ExtractAttributeFromTagLeftOf(kAttrinuteAlign);

  string line = "| ";

  if (align == "left" || align == "center")
    line += ':';

  line += '-';

  if (align == "right" || align == "center")
    line += ": ";
  else
    line += ' ';

  c->tableLine.append(line);

  c->appendToMd("| ");
}

void Converter::TagTableHeader::OnHasLeftClosingTag(Converter *c) {}

void Converter::TagTableData::OnHasLeftOpeningTag(Converter *c) {
  if (c->prev_prev_ch_in_md_ != '|')
    c->appendToMd("| ");
}

void Converter::TagTableData::OnHasLeftClosingTag(Converter *c) {}

void Converter::TagBlockquote::OnHasLeftOpeningTag(Converter *c) {
  ++c->index_blockquote;
}

void Converter::TagBlockquote::OnHasLeftClosingTag(Converter *c) {
  --c->index_blockquote;
  c->ShortenMarkdown(2); // Remove the '> '
}

void Converter::reset() {
  md_.clear();
  prev_ch_in_md_ = 0;
  prev_prev_ch_in_md_ = 0;
  index_ch_in_html_ = 0;
}

bool Converter::IsInIgnoredTag() const {
  if (current_tag_ == kTagTitle && !option.includeTitle)
    return true;

  return IsIgnoredTag(current_tag_);
}
} // namespace html2md
