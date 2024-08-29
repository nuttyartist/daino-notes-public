# html2md

[TOC]

## What does it do

html2md is a fast and reliable C++ library for converting HTML content into markdown. It offers support for a wide range of HTML tags, including those for formatting text, creating lists, and inserting images and links. In addition, html2md is the only HTML to markdown converter that offers support for table formatting, making it a valuable tool for users who need to convert HTML tables into markdown.

## How to use this library

### CMake

Install html2md. Either use the pre-built packages found on [GitHub releases](https://github.com/tim-gromeyer/html2md/releases) or build and install it yourself.


Afterwards:

```cmake
find_package(html2md)
target_link_library(your_target PRIVATE html2md)
```

### Manually

To use html2md, follow these steps:

1. Clone the library: `git clone https://github.com/tim-gromeyer/html2md`
2. Add the files `include/html2md.h` and `src/html2md.cpp` to your project
3. Include the `html2md.h` header in your code
4. Use the `html2md::Convert` function to convert your HTML content into markdown

Here is an example of how to use the `html2md::Convert` function:

```cpp
#include <html2md.h>

//...

std::cout << html2md::Convert("<h1>foo</h1>"); // # foo
```

## Supported Tags

html2md supports the following HTML tags:


| Tag          | Description        | Comment                                    |
| ------------ | ------------------ | ------------------------------------------ |
| `a`          | Anchor or link     | Supports the `href` and `name` attributes. |
| `b`          | Bold               |                                            |
| `blockquote` | Indented paragraph |                                            |
| `br`         | Line break         |                                            |
| `cite`       | Inline citation    | Same as `i`.                               |
| `code`       | Code               |                                            |
| `dd`         | Definition data    |                                            |
| `del`        | Strikethrough      |                                            |
| `dfn`        | Definition         | Same as `i`.                               |
| `div`        | Document division  |                                            |
| `em`         | Emphasized         | Same as `i`.                               |
| `h1`         | Level 1 heading    |                                            |
| `h2`         | Level 2 heading    |                                            |
| `h3`         | Level 3 heading    |                                            |
| `h4`         | Level 4 heading    |                                            |
| `h5`         | Level 5 heading    |                                            |
| `h6`         | Level 6 heading    |                                            |
| `head`       | Document header    | Ignored.                                   |
| `hr`         | Horizontal line    |                                            |
| `i`          | Italic             |                                            |
| `img`        | Image              | Supports the `src` and `alt` attributes.   |
| `li`         | List item          |                                            |
| `meta`       | Meta-information   | Ignored.                                   |
| `ol`         | Ordered list       | Don't use other lists in this list.        |
| `p`          | Paragraph          |                                            |
| `pre`        | Preformatted text  | Works only with `code`.                    |
| `s`          | Strikethrough      | Same as `del`.                             |
| `span`       | Grouped elements   |                                            |
| `strong`     | Strong             | Same as `b`.                               |
| `table`      | Table              |                                            |
| `td`         | Table data cell    | Uses `align` from `th`.                    |
| `th`         | Table header cell  | Supports the `align` attribute.            |
| `title`      | Document title     | Same as `h1`.                              |
| `tr`         | Table row          |                                            |
| `u`          | Underlined         | Uses HTML.                                 |
| `ul`         | Unordered list     |                                            |

## Bindings

- [Python](../python/README.md)

## Requirements

1. A compiler with **c++11** support like *g++>=9*

That's all!

## License

html2md is licensed under [The MIT License (MIT)](https://opensource.org/licenses/MIT)
