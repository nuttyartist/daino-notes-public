# Code Example Markdown

## Python Code

You can include Python code blocks like this:

```python
def factorial(n):
    if n == 0:
        return 1
    else:
        return n * factorial(n - 1)

result = factorial(5)
print("Factorial of 5:", result)
```

## JavaScript Code

JavaScript code can be included like this:

```javascript
function fibonacci(n) {
    if (n <= 1) {
        return n;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

const fibResult = fibonacci(6);
console.log(`Fibonacci of 6: ${fibResult}`);
```

## Inline Code

You can also include inline code using backticks. For example, `print("Hello, World!")` is a simple Python print statement.

## Syntax Highlighting

Markdown supports syntax highlighting for various programming languages, making your code more readable. For instance, you can specify the language after the triple backticks:

```java
public class HelloWorld {
    public static void main(String[] args) {
        System.out.println("Hello, World!");
    }
}
```

Enjoy using code snippets in your Markdown files!
