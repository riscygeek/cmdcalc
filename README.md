# Commandline-Calculator
A simple command line calculator

# How to install
Windows: <br>
Compile with Visual Studio

Linux/WSL: <br>
<code>gcc -o cmdcalc *.c -lm </code>

# Using cmdcalc
- Expressions (like <code>1 + 2 * a / f(42)</code>)
- User-defined functions (like <code>fun add3(a, b, c) = a + b + c</code>)
- List all Variables with <code>vars</code>
- List all Functions with <code>funcs</code>

# How to embed cmdcalc in your project
Just modify console.c and remove main.c to fit to your project.
