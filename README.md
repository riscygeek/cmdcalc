# Commandline-Calculator
A simple command line calculator

# Dependencies
- GNU Readline
- getopt.h

# How to install
Windows: <br>
Compile with Visual Studio

Arch Linux/Manjaro: <br>
Install ```cmdcalc``` with package manager<br>

Linux: <br>
Compile with CMake

# Using cmdcalc
- Expressions (<code>1 + 2 - 3 * a ** f(42 / 2)</code>)
- Variables (<code>a=99</code>)
- Functions (<code>func f(x)=x*2</code>)
- Arrays (<code>arr=append(arr, 32, "Hello World", ["another array"])</code>)

# Operators
- \+ Addition
- \- Subtraction
- \* Multiplication
- ** Power
- / Division
- = Assignment
- condition ? true : false

# Interpreter-specific commands
- vars (list variable names)
- funcs (list function names)
- CTRL+D exit
- CTRL+C discard command
- func name(param1, param2)="definition"

# TODO
- Implement file execution
- Add useful command-line options
- Add Documentation/Wiki
- Find and fix bugs
- Add more functions
- Add complex statements (like if, for)
