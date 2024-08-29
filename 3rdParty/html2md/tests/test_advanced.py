import pyhtml2md

options = pyhtml2md.Options()
options.splitLines = False

converter = pyhtml2md.Converter("<h1>Hello Python!</h1>", options)

def test_main():
    assert converter.convert() == "# Hello Python!\n"
    assert converter.ok() == True

