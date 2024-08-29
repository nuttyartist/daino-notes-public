import pyhtml2md

def test_main():
    assert pyhtml2md.convert("<h1>Hello, world!</h1>") == "# Hello, world!\n"

