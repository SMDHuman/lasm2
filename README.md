<img src="res/LOTP Assembler 2 Logo.png" width="256" alt="LOTP Assembler 2">

# LOTP Assembler 2
This is a successor to a custom assembler of mine called LASM. This new version aims to change some implementations of the main codebase and tries to make adding new Instruction Sets much more pleasant. LASM2 is not backwards compatible. Some new features makes the usage of this version of assembler much more intuitive than the previous version. 

### What has special than other assemblers?
It has all the necessary macros that we love from the C language, namespaces for branch labels, and support for numbers of arbitrary (effectively unlimited) size. All of these are to make this language fun to use and to try new methods of writing assembly.
## How To Use
  Clone the project and run `make` on your terminal to build.
  ```bash
  git clone https://github.com/SMDHuman/lasm2.git
  cd lasm2
  make install
  lasm2 --help
  ``` 
  Currently you only need to give the one input file and a machine name to assemble a program.
  ```bash
  lasm2 examples/basic_syntax.l -m 6502
  ```
  You can optionally add an output path for the binary
  ```bash
  lasm2 examples/basic_syntax.l -m 6502 -o bin/basic_syntax.bin
  ```
## Features 
### Comments and New Lines
You can write comments with the good old ```//``` double slash. Usually, you do not need to use ```;``` at the end of your lines. New lines are sufficient, but if you want to write multiple statements on one line, you can separate them with semicolons.
```
// My comment on this code is NULL
lda# 3; sta 0
adc 1; sta 1 //Things happen
```

### Macros
Macros are defined inside ```<# #>``` structure. There are 4 main types of macros: Include, Define, Define With Arguments and If Define, as we know them from C89.
```
// INCLUDE - insert entire code from file
<#"path/to/filename.l"#>

// DEFINE - whenever assembler sees 'recall_name', it replaces with definition
<#recall_name; and other things#>
recall_name; recall_name

// DEFINE WITH ARGS - positional arguments 
<#recall_name arg_name another_arg;
  and other things arg_name with another_arg
#>
recall_name (fizz, buzz)

// IF DEFINE - controller for macro parser to enable or block code
<#?this can do this#>      // if 'this' is defined
<#!that <#that define#>#>    // if 'that' not defined
```

### Everything is Data
Everything that doesn't belong to instruction words will be parsed as an expression. Assemblers convert keywords and number representations into bytes to upload to memory.
```
// Different number formats and expressions
123; 0xfe; 0b01011
3 + (2 - 3) * (0x12 << 2) / 3
"Hello World!"

// Manipulate values with special operations
"text with words"[2]  // Isolates character 'x' by index
"Hello Code"[3:5]     // Isolates character 'lo' by range of indexes
0xfff $4              // Ensure value takes 4 bytes of space
3 + (2$32) - 1        // This is equal to '4' but occupies 32 bytes. Size of bytes inherits though evaluation
```

### Labels, Namespaces & Memory Management
**Branch Labels** have two properties: constant and undetermined. You can call future branches from previous lines, and undetermined labels are evaluated when the assembler parses them.
```
// Determined labels - fixed values
x[0]: y[x+1]: z[y+2]:

// Can reference labels before definition
start[0x8000 :: start + 0x200]:
  lda_i 0x42
  loop: sta x; adc 1; bcc loop
  jmp end
end[0xff00 + loop]: hlt
```
> **_NOTE:_** Determined labels can be any size of byte but undetermined labels fixed to machine specific byte size.

**Namespaces & Scopes** isolate labels using curly braces '{}'. Code can reach upper levels but not lower ones. A named scope creates a namespace to make it accessible afterwards.
```
zp{
  x[0]: y[2]: z[4]:
  stack[0x100]{ start: end[start+0x255]: }
}

start[0x8000]{ sta zp.x }
```

**Memory Restrictions** use range function to restrict code to specific memory locations. When you create a branch label with a range, the assembler will error if the limit is exceeded.
```
zp[0::256]{ x: 0 .(50) }                    // Restrict to 256 bytes
start[0x8000 :: start+0x400]:               // Range from 0x8000 to 0x8400
subroutines[0x9000 :: 0xA000]: // Some code
``` 
## Adding Instruction Sets
You can add new functions and instructions with just using the same macro tools as you use your assembly code.
```
// 6502 Add Memory to Accumulator with Carry Instruction Implementation
<#ADC_I _i; 0x69; _i#>
<#ADC ad;
  ?($ad == 1) 0x65; ad
  ?($ad == 2) 0x6D; ad$2
  ?($ad => 2) <#ERROR; address is too log#>
#>
<#ADC_X ad; 
  ?($ad == 1) 0x75; ad
  ?($ad == 2) 0x7D; ad$2
  ?($ad => 2) <#ERROR address is too log#>
#>
<#ADC_Y ad; 0x79; ad$2 #>
<#ADC_IX ad; 0x61; ad$2 #>
<#ADC_IY ad; 0x71; ad$2 #>
```

### What "LOTP" means?
LOTP is an acronym that I use to give my project's name as a signature of mine. It means "Line On The Paper". From the beginning of my engineering and maker journey, I usually start a project on a paper with some sketches and ideas. It refers to the root of the starting point of my projects.