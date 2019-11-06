# ire
This repository contains the source code for irec, the ire compiler.


### What is implemented so far (as of 0.3.0)
  - functions
  - variables
  - type system: native types (Int, Byte, Boolean), arrays, product types, sum types (tagged unions)
  - pattern matching on unions via the pattern match operator (:)
  - imports (very basic)
  - arithmetic
  - conditional branching (if, elseif, else)
  - logical operators (and, or)
 
### What will be implemented in the future
  - rewrite of import system
  - standard library
  - gc 
  - first class functions, lambdas, closures
  - other os support (mac, windows)

### Usage and options
usage: irec \[options\] file

irec -h for options. by default "irec file.ire" is equivalent to "irec file.ire -bllvm -O0 -o file"

### Examples
See the tests directory. the tests can be run by typing "make test"

note that irec is not very stable right now, and can sometimes crash on bad input.
