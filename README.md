hdrone
======
**Forked from [ndrone-experiment](https://github.com/loyd/ndrone-experiment) to continue development.**  
Hybrid software for quadrocopter on Node.js.  

![Diagram](http://troloload.ru/f/22767_22766_ecveafynvdeeta9w.png)  
¹ state — data block, contains navigation (attitude) and conditional (temperatures, load average, memory and cpu usage) data;  
² video — continuous video stream.

Committing
==========
Commit style
------------
[Follow Google's AngularJS](https://docs.google.com/document/d/1QrDFcIiPjSLDn3EL15IJygNPiHORgU1_OOAqWjiDU5Y/edit#).

Commit style propositions
-------------------------
1. If subject has file/directory, then path shall lead to nearest parent directory/itself.  
2. All file extensions shall be specified, but sources: `.ts`, `.cc`, `.hh`.
3. Subject name shall be kept as it will be used in code

Algorithm to commit work
------------------------
1. If work was done in one commit then `push` to `master`.  
2. If word requires more than one commit then:  
  2a. create branch `{work name}`;  
  2b. commit message conventions are still actual within branches;  
  2c. when work is done, do `pull` and `rebase` with `master`;  
  2d. `merge` with `--edit --no-ff` and name it according to objectives (`feat/fix/chore`) `({work subject}): {work description}`;  
  2e. then `push`.
  
Coding
======

Order of items within classes
-----------------------------
* static fields
* static accessors
* static methods
* instance fields
* instance accessors
* constructor
* instance methods

Within each of these groups order by access:  
* public
* private

Code style
----------
TypeScript: follow [Google's JavaScript guide](https://google-styleguide.googlecode.com/svn/trunk/javascriptguide.xml).
C/C++: follow [Google's C++ guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml).

Code style propositions
-----------------------
1. Do not use `@const` (TypeScript)
2. Do not use `using namespace` (C/C++)
2. Use constructions like `if(...)`, `while(...)`
3. Make `file_names_like_this`
4. `lowerCamelCase` (TypeScript)
5. `snake_case` (C/C++)
6. Identation in 4 whitespaces
7. 80-90 symbols per line
8. Always align multiline arguments or conditions for readability
9. Do not use `{}` for single operators
10. Prototype extentions in `libs/` only
11. always align `:` or `=` by logical blocks and at minimum difference
12. `:` without whitespace after variable, but before type (TypeScript)
13. Name imported modules equal to they real names
14. use `//#TODO: ...`, `//#FIXME: ...` and `//#XXX: ...`

Documenting
===========
Documentation style
-------------------
Follow [Google's JSDoc](https://developers.google.com/closure/compiler/docs/js-for-compiler)
