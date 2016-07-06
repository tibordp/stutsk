Stutsk is a general-purpose concatenative programming language. It is similar to FORTH and Postscript
in syntax, similar to Lisp in philosophy and similar to PHP in lexicon and semantics.

Try out [the sandbox][sandbox]!

  [sandbox]: http://www.ojdip.net/stutsk-online/

Author: Tibor Djurica Potpara <tibor.djurica@ojdip.net>

Prerequisites:
--------------
* Boost libraries - <http://www.boost.org/> (tested with 1.51, any recent version will do)
* Crypto++ library - <http://www.cryptopp.com/> (tested with 5.6.1, should work with more recent versions)
* libexecstream - <http://libexecstream.sourceforge.net/> (included in the source tree)
* GNU Make
* A C++11-compliant compiler (GCC 4.6+ should work)

How to build:
-------------
If all prerequisites are satisfied, `stutsk` can be built with  
    
    make
    
Installation:
-------------

As `stutsk` consists of a single executable file, it can be installed simply by copying the executable
to a directory included in `PATH`


Usage:
------

Basic usage:

    stutsk <input_file>
    
As lines beginning with `#` are line comments in `stutsk`, you can also use shebangs in your scripts.

    #!/path/to/stutsk
    "Hello world" print
    
See source files for function reference.

Code examples:
--------------

Because code tells more than 1000 words of description, here's a motivating example

Obligatory example:

    "Hello world" print 
    
Recursive factorial:
    
    { 
      $n = 
      $n 1 == { 1 } { $n $n 1 - recurse * } ifelse      
    } "factorial" function
    
    5 factorial print # Prints 120
    
Calculate 100 digits of Pi (deliberatly obfuscated):
    
    100

    126 chr dup dup . { $c global 10 $c 1 + *
    3 div } swp function { dup 0 > { 2 * 1 + }
    { dmp 10 } ifelse } swp function { $i static
    $i is_def ! { 0 $i = } if $i ++ $i 1 == {
    dmp exit 12 } if $i 3 == { 46 chr print } if
    print 1 dmp } 92 chr function 124 { { repeat
    } inherit 0 } swp chr function { { $s $i }
    inherit } swp $c = 4 swp 35 chr function 1 .
    
    ( ~~ 1 - { 2 } | 2 + ) $r
    = ( ~~ 1 - { 0 } | ) $s =
    0 $r ~~ 1 - [] $s ~~ 1 -
    [] = 0 $n = $p = $c 1 +
    { ~~ 2 - $i = ~~ 1 - { $i
    1 + # 1 + [] @ $i 1 + ~
    div * $r $i [] 10 * + #
    [] = # [] @ $i ~ % $r $i
    [] = $i -- } | $s swp []
    @ 0 ~ div $q = $q 9 == {
    $n ++ } { $q 10 == { $p 1
    + \ $n { 0 \ } | $n = 0
    $p = } { $p \ $q $p = $n
    0 != { $n { 9 \ } | $n =
    } if } ifelse } ifelse }
    
    repeat
