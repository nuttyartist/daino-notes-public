## How does the test work?

Well, the program searches(in this dir) for files ending with `.md`.

1. It then converts the Markdown to HTML using [md4c](https://github.com/tim-gromeyer/MarkdownEdit_md4c).
2. Afterwards it converts the HTML back to Markdown. 
3. The generated Markdown gets converted back to HTML
4. It compares the HTML generated from the original Markdown  
and the HTML generated from the converted Markdown.
