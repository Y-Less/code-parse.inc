# code-parse.inc

## Introduction

This library is designed for generating code at compile-time based on existing
code.  It can be thought of as a macro equivalent to amx-assembly - used for
advanced manipulation of the compiler for creating libraries and keywords, more
than end-user code.  As a simple example we will create a macro to count the
number of array parameters in a function, walking through each step in turn.
After that we will demonstrate a more in-depth example to create a y_timers
clone.

## Example 1 - Count Array Parameters

### Full Code

```pawn
// The entry point.  Defines our parser.
#define ARRAY_COUNT:%1(%2) FUNC_PARSER(ARRAY_COUNT,ARR:NUM:)(%1(%2)) stock const countOf_%1 = 0; %1(%2)

// Default.  Called for parameters that didn't match any other type.
#define ARRAY_COUNT_NUM(%9)%8$ %8$

// Arrays.  Called when an array is matched.
#define ARRAY_COUNT_ARR(%9)%8$%0=%1; %8$%0=%1 + 1;

// Ending.  Called when the parsing is complete.
#define ARRAY_COUNT_END(%9)%8$ %8$

// Ending.  Called when there are no function parameters.
#define ARRAY_COUNT_NUL(%9)%8$ %8$

// Use the macro on a function.
ARRAY_COUNT:my_test_function(a, b[], c, const d[], &e = 6, string:f[] = "hi")
{
}

main()
{
	printf("There are %d array parameters to \"my_test_function\"", countOf_my_test_function);
}
```

### Console Output

```
There are 3 array parameters to "my_test_function"
```

### Explanation

To understand the code, we need a clear idea of the code inputs and outputs (and
a good understanding of the pre-processor helps as well).

This is the code input (input):

```
my_test_function(a, b[], c, const d[], &e = 6, string:f[] = "hi")
```

This is the code output (output):

```
stock const countOf_my_test_function = 0 + 1 + 1 + 1;
my_test_function(a, b[], c, const d[], &e = 6, string:f[] = "hi")
```

The bit of the output we really care about is just the first part:

```
stock const countOf_my_test_function = 0 + 1 + 1 + 1;
```

This starts out as:

```
stock const countOf_my_test_function = 0;
```

And has ` + 1` appended every time an array parameter is encountered, with other
parameters getting ignored (`0 + 1 + 1 + 1` can be done by the compiler, so will
only generate `3` in the AMX, not a string of sums).

The library handles the hard part of understanding all the function parameter
types (the input), but you still need to understand and split up your own code
(the output).

Line By Line

```pawn
#define ARRAY_COUNT:%1(%2) FUNC_PARSER(ARRAY_COUNT,ARR:NUM:)(%1(%2)) stock const countOf_%1 = 0; %1(%2)
```

```pawn
#define ARRAY_COUNT:%1(%2)
```

This is just the macro name and match as normal.  Will match anything that looks
like a function with a tag of `ARRAY_COUNT:`.

```pawn
FUNC_PARSER(ARRAY_COUNT,ARR:NUM:)(%1(%2))
```

Create a new function parser called `ARRAY_COUNT`, define which parameter types
it will analyse, and pass it the input.  The parameter type parsers are all
followed by colons and not separated by commas (or any spaces - that is VERY
important).

**DO NOT PUT ANY SPACES IN THIS PART!**

`ARR:` will detect any array.  `NUM:` is essentially "other" - it will detect
anything not detected by any other parser.  There is also `EXT:` (extended)
which will detect `...`, `REF:` (reference) which will detect `&a`, and `STR:`
(string) which will detect `string:a[]`.  This last point is important - because
strings look exactly like arrays (`a[]` and `a[]` - because they ARE arrays) but
are frequently handled differently (for example using `s` instead of `a` in
`CallRemoteFunction`) this library uses the YSI convention of prefixing all
strings with the `string:` tag.  This does not affect any other code in ANY way
- you won't even get any tag mismatch warnings because the tag is removed again
later on in the compilation process.  All it means is that the two types can be
differentiated.

### Exclude Strings

To modify the example so that strings are not included in the array count would
look like:

```pawn
#define ARRAY_COUNT:%1(%2) FUNC_PARSER(ARRAY_COUNT,ARR:NUM:STR:)(%1(%2)) stock const countOf_%1 = 0; %1(%2)
#define ARRAY_COUNT_STR(%9)%8$ %8$
```

`ARRAY_COUNT` is used as a prefix to all other macros, so is prepended whenever
a macro callback is called (with a `_` separator):

```pawn
#define ARRAY_COUNT_NUM(%9)%8$ %8$
```

`ARRAY_COUNT` is the custom prefix specified in the `FUNC_PARSER` call.  `_NUM`
is the suffix for any normal number variables (or anything not otherwise
matched).  `(%9)` is the parameters for this macro - things like name of the
variable, array size, default values, etc.  These are not used here, so they can
just be ignored.  `%8$` is required in both the input and the output, but is
exclusively part of the parser and is what makes the iteration work.  The
contents of `%8` are just the current parser state, `$` is the delimiter after
which comes your stuff (if desired, here it isn't).

```pawn
#define ARRAY_COUNT_ARR(%9)%8$%0=%1; %8$%0=%1 + 1;
```

This is a more complex callback.  Everything up to the `$` is the same -
parameters and internal state, with the `%8$` also appearing in the output
because it is always required (not putting that there will just break the
compilation).

In the `#define ARRAY_COUNT:` line, the first thing that came after the call to
`FUNC_PARSER` was ` stock const countOf_%1 = 0;`.  This is the start of user-
generated output, and is what now appears after the `$`.  So because we want to
change this to ` stock const countOf_%1 = 0 + 1;`, we match against that code
with: `%0=%1;` (technically even the `%0=` isn't required as `%1;` will just
match everything up to the first semi-colon).  Copying that to the output gives
`%0=%1 + 1;`.  We don't need the function definition at the very end, so there's
no point matching against it.

```pawn
#define ARRAY_COUNT_END(%9)%8$ %8$
```

Called when the parsing is complete.

```pawn
#define ARRAY_COUNT_NUL(%9)%8$ %8$
```

Called when there are no function parameters.

## Example 2 - Count All Parameters

We can extend the example again to count all parameter types.  For this, we will
set up some naming rules first:

Substitution parameter | Use
--- | ---
`%0` | The current reference count (`&a`).
`%1` | The current varargs count (`...`) - should only be 0 or 1 but the parser doesn't check that, it can parse quite incorrect code.
`%2` | The current array count (`a[]`).
`%3` | The current string count (`string:a[]`).
`%4` | The current other count (`a`).
`%8` | The parser state (black box).
`%9` | Unused other parameters.

```pawn
// Entry.  Parse all parameter types.
#define ALL_COUNT:%1(%2) FUNC_PARSER(ALL_COUNT,ARR:NUM:EXT:STR:REF:)(%1(%2)) \
	stock const \
		refCount_%1 = 0, \
		extCount_%1 = 0, \
		arrCount_%1 = 0, \
		strCount_%1 = 0, \
		numCount_%1 = 0; \
	%1(%2)

// Counts.  Add one to the relevant number.
#define ALL_COUNT_REF(%9)%8$%0,%1,%2,%3,%4; %8$%0 + 1,%1,%2,%3,%4;
#define ALL_COUNT_EXT(%9)%8$%0,%1,%2,%3,%4; %8$%0,%1 + 1,%2,%3,%4;
#define ALL_COUNT_ARR(%9)%8$%0,%1,%2,%3,%4; %8$%0,%1,%2 + 1,%3,%4;
#define ALL_COUNT_STR(%9)%8$%0,%1,%2,%3,%4; %8$%0,%1,%2,%3 + 1,%4;
#define ALL_COUNT_NUM(%9)%8$%0,%1,%2,%3,%4; %8$%0,%1,%2,%3,%4 + 1;

// Endings
#define ALL_COUNT_END(%9)%8$ %8$
#define ALL_COUNT_NUL(%9)%8$ %8$
```

The first macro is spread over multiple lines for clarity, but doesn't have to
be.

Use:

```pawn
ALL_COUNT:all_test_function(a, b[], c, const d[], &e = 6, string:f[] = "hi")
{
}

main()
{
	printf("There are %d reference parameters to all_test_function", refCount_all_test_function);
	printf("There are %d extended parameters to all_test_function",  extCount_all_test_function);
	printf("There are %d array parameters to all_test_function",     arrCount_all_test_function);
	printf("There are %d string parameters to all_test_function",    strCount_all_test_function);
	printf("There are %d other parameters to all_test_function",     numCount_all_test_function);
}
```

## Example 3 - Count Array Parameters And Sum Their Size

Lets go back to the first example and modify it to show not just the number of
array parameters, but also their combined size.  First a side-note - function
array parameters are often written as `[]`, which we will count as a size of 0.
However, writing `1 +  + 10` is not valid syntax - we actually need a number
there.  Writing `(size +0)` when there is a size will resolve to `(size)`, when
there isn't a size it will resolve to `(+0)`, the unary plus and legal, giving
`(1+0) + (+0) + (1+0)`.  This is the simplest method of dealing with potentially
empty macros.

How do we get the size though?

Previously the addition macro was:

```pawn
#define ARRAY_COUNT_ARR(%9)%8$%0=%1; %8$%0=%1 + 1;
```

The `(%9)` was a placeholder for parameters we didn't care about - it will
consume them all and drop them.  However, each of the `_ARR`, `_STR`, `_NUM`,
`_EXT`, and `_REF` macros take several parameters:

Substitution parameter | Use
--- | ---
`%0`  | `const ` (if it exists).
`%1`  | The tag (if there is one - and only the first if there are many, includes the `:`).
`%2`  | The variable name (only always present value).
`%3`  | Length (strings only).
`%4`  | Default value (if it exists, not on arrays).
`%3+` | Dimension sizes (arrays only).

These parameters:

```pawn
const Float:pos[3]
string:name[] = "Bob"
&v
Float:v = 0
```

Would call as:

```pawn
ARRAY_COUNT_ARR(const ,Float:,pos,3)
ARRAY_COUNT_STR(,,name,,"Bob")
ARRAY_COUNT_REF(,,v,)
ARRAY_COUNT_NUM(,Float:,v,0)
```

Note that the `string:` tag does NOT get passed - it is special and purely used
for internal differentiation.

We can thus change the macro to use one parameter and just ignore the others:

```pawn
#define ARRAY_COUNT_ARR(%0,%1,%2,%3)%8$%6;%7; %8$%6 + 1;%7 + (%3 +0);
```

If you didn't know, you can also use the same substitution variable multiple
times in one macro, with the value getting overridden each time.  I like to do
this on values that are getting ignored, with `%9` for it:

```pawn
#define ARRAY_COUNT_ARR(%9,%9,%9,%4)%8$%0;%1; %8$%0 + 1;%1 + (%4 +0);
```

This makes the full thing:

```pawn
// The entry point.  Defines our parser.
#define ARRAY_COUNT:%1(%2) FUNC_PARSER(ARRAY_COUNT,ARR:NUM:)(%1(%2)) stock const countOf_%1 = 0; stock const sizeOf_%1 = 0; %1(%2)
#define ARRAY_COUNT_NUM(%9)%8$ %8$
#define ARRAY_COUNT_ARR(%9,%9,%9,%3)%8$%0;%1; %8$%0 + 1;%1 + (%3 +0);
#define ARRAY_COUNT_END(%9)%8$ %8$
#define ARRAY_COUNT_NUL(%9)%8$ %8$

// Use the macro on a function.
ARRAY_COUNT:my_test_function(a, b[], c, const d[], &e = 6, string:f[] = "hi")
{
}

main()
{
	printf("There are %d array parameters with size %d to \"my_test_function\"", countOf_my_test_function, sizeOf_my_test_function);
}
```

## Example 4 - Auto-Generate `CallRemoteFunction` Specifier Strings

Time for a more advanced example, using the parameter names more.  The end
result will be a new keyword `remote` which behaves like `public`, but also
generates code to call `CallRemoteFunction` directly with the correct string
specifier for out function.  Doing:

```pawn
remote my_func(a, b[], c)
{
}
```

Will generate:

```pawn
my_func(a, b[], c) CallRemoteFunction("remote_my_func", "iai", a, b, c);
forward remote_my_func(a, b[], c);
public remote_my_func(a, b[], c)
{
}
```

Thus just doing: `my_func(10, arr, sizeof (arr));` in code will call all scripts
at once, not just this one.

A normal macro would not be able to determine the exact string `iai` to use in
this situation, and just using the parameters directly in `CallRemoteFunction`
would result in the invalid call `a, b[], c`.

First, we define the entry point and bulk of the output:

```pawn
#define remote%0(%1) FUNC_PARSER(REMOTE,ARR:STR:NUM:)(%0(%1)) \
	%0(%1) CallRemoteFunction("remote_"#%0, ##); \
	forward remote_%0(%1); \
	public remote_%0(%1)
```

We also define a helper macro to turn `remote_ my_func` in to `remote_my_func`:

```pawn
#define remote_%0\32; remote_%0
```

For each of the parameter types, we add a letter to the end of the specifier
string, and add the parameter name to the end of the `CallRemoteFunction`
parameter list:

```pawn
#define REMOTE_ARR(%9,%9,%2,%9)%8$%0(%1)%3(%4#%5#%6#%7) %8$%0(%1)%3(%4#%5#%6a#%7,%2)
#define REMOTE_STR(%9,%9,%2,%9)%8$%0(%1)%3(%4#%5#%6#%7) %8$%0(%1)%3(%4#%5#%6s#%7,%2)
#define REMOTE_NUM(%9,%9,%2)%8$%0(%1)%3(%4#%5#%6#%7) %8$%0(%1)%3(%4#%5#%6i#%7,%2)
```

You could abstract this further, since the only tiny difference between those
three macros is the letter inserted at the end between the two `#`s.  The reason
why `%0(%1)%3(%4#` was used instead of just `%0#` to skip over everything up to
the first `#` is that the `#` is inside some brackets, and they are handled very
specially by the pre-processor unless you explicitly match against the `(`.

Finally, we need the two end macros because they are required but don't do
anything in this instance:

```pawn
#define REMOTE_END(%9)%8$ %8$
#define REMOTE_NUL(%9)%8$ %8$
```

### Three Facts

1. The `forward` keyword is optional.

2. A public function with only a semi-colon after it is a forward.

3. An `@` prefix declares a function public.

We can thus shrink this code to:

```pawn
#define remote%0(%1) FUNC_PARSER(REMOTE,ARR:STR:NUM:)(%0(%1)) stock%0(%1)CallRemoteFunction("@r_"#%0,##);@r_%0(%1);@r_%0(%1)
#define @r_%0\32; @r_
```

As a side-effect of point interesting fact 2, writing this:

```pawn
remote truly_remote(x, y, z);
```

Will declare the function but not define it, allowing a function in a different
script to be easilly called from here.

### Tags

You may have noticed that all normal variables are passed as `i`, never `f`.
PAWN is typeless - the tags are only a hint to the compiler, and only matter at
all for some operators, which can be overloaded based on tag.  Using `i` instead
of `f` makes no difference at all here.  To be really strict you could use:

```pawn
#define REMOTE_NUM(%9,%9,%2)%8$%0(%1)%3(%4#%5#%6#%7) %8$%0(%1)%3(%4#%5#%6i#%7,_:%2)
```

Detecting only the `Float:` tag and leaving all others is much more involved,
and just not worth it.

## Example 5 - Output Input Exactly

For the next example, we are going to write a set of macros that perfectly
output their input...  This sounds pointless - and in code it would be, but is
important here for demonstrating every parameter of every callback macro,
including some I slightly lied about previously (or rather simplified).  The
first lie was about default parameters.

### Lie 1

Earlier I stated that:

```pawn
const Float:pos[3]
string:name[] = "Bob"
&v
Float:v = 0
```

Gave:

```pawn
ARRAY_COUNT_ARR(const ,Float:,pos,3)
ARRAY_COUNT_STR(,,name,,"Bob")
ARRAY_COUNT_REF(,,v)
ARRAY_COUNT_NUM(,Float:,v,0)
```

In actual fact, when a default value is present an extra bit is appended to the
macro name to aid in processing.  So the real output is:

```pawn
ARRAY_COUNT_ARR(const ,Float:,pos,3)
ARRAY_COUNT_STR_DEF(,,name,,"Bob")
ARRAY_COUNT_REF(,,v)
ARRAY_COUNT_NUM_DEF(,Float:,v,0)
```

Note that `const ` has a space after it (the reason for which will be seen
shortly).

The other potential suffix is for multi-dimensional arrays:

```pawn
const Float:pos[][3][MY_ENUM]
```

Becomes:

```pawn
ARRAY_COUNT_ARR_ARR_ARR(const,Float:,pos,,3,MY_NUM)
```

The number of `_ARR` suffixes determine the number of dimensions.

The parameters might be a little confusing, so:

```pawn
PREFIX_NUM(const, tag, name)
PREFIX_NUM_DEF(const, tag, name, default)
PREFIX_REF(, tag, name)
PREFIX_REF_DEF(, tag, name, default)
PREFIX_EXT(, tag,)
PREFIX_STR(const,, name, size)
PREFIX_STR_DEF(const,, name, size, default)
PREFIX_ARR(const, tag, name, size_1)
PREFIX_ARR_ARR(const, tag, name, size_1, size_2)
PREFIX_ARR_ARR_ARR(const, tag, name, size_1, size_2, size_3)
PREFIX_ARR_ARR_ARR_ARR(const, tag, name, size_1, size_2, size_3, size_4)
```

Reference parameters can never be `const ` (that makes no sense), so they will
never have `const ` passed.  However, they still have the parameter there but it
is safe to ignore.  Same with extended (vararg) parameters, which can also not
have default values so there is no `_DEF` variant of them.  For everything else
the `const ` is optional - it may be there or it may be a blank parameter.  As
we will see, often it doesn't matter either way.  Finally, anything more than
two dimensions in an array and the user should probably rethink everything
anyway, so don't worry about the really long ones.  Strings have the tag
parameter but never use it, and can only ever have one dimension.

### Lie 2

The other simplification was that all of these variations are detected for you -
they are not, they must be requested.  Each type (with/without const, defaults,
etc) requires extra macros.  It is always STRONGLY recommended that you use the
updated compiler where this doesn't matter at all, but for very complex macros
on the default compiler, long functions with many options may blow past the line
length limit.  Also, for some contexts it may not matter - public functions
can't have default values etc, so why bother scanning for them doing extra work
that isn't required?  The options are:

Name | Use
--- | ---
TAG | Look for tags.
MUL | Allow multi-dimensional arrays (more than one dimension).
CST | Look for "const".
DEF | Look for defaults.

The earlier parsers shown didn't use any of these, as often they weren't needed
because the actual values weren't used.  A parser that would detect every
combination of parameter types would look something like:

```pawn
FUNC_PARSER(REBUILD,ARR_MUL_TAG_CST:NUM_TAG_CST_DEF:REF_TAG_DEF:EXT_TAG:STR_CST_DEF:)(input)
```

There are also longer synonyms if you prefer clarity:

```pawn
FUNC_PARSER(REBUILD,ARRAY_TAG_CONST_MULTI:NUMBER_CONST_TAG_DEFAULT:TAG_REFERENCE_DEFAULT:TAG_VARARG:DEFAULT_STRING_CONST:)(input)
```

Clearly, the order of parts doesn't matter - "ARR_MUL_CST" and "CST_MUL_ARR" are
the same.

### Output One Parameter

The code to output one parameter exactly as it came in would be:

```pawn
#define REBUILD_NUM(%0,%1,%2)%8$                         %8$%0 %1%2
#define REBUILD_NUM_DEF(%0,%1,%2,%4)%8$                  %8$%0 %1%2     = %4
#define REBUILD_REF(,%1,%2)%8$                           %8$ & %1%2
#define REBUILD_REF_DEF(,%1,%2,%4)%8$                    %8$ & %1%2     = %4
#define REBUILD_EXT(,%1,)%8$                             %8$   %1...
#define REBUILD_STR(%0,,%2,%3)%8$                        %8$%0   %2[%3]
#define REBUILD_STR_DEF(%0,,%2,%3,%4)%8$                 %8$%0   %2[%3] = %4
#define REBUILD_ARR(%0,%1,%2,%3)%8$                      %8$%0 %1%2[%3]
#define REBUILD_ARR_ARR(%0,%1,%2,%3,%4)%8$               %8$%0 %1%2[%3][%4]
#define REBUILD_ARR_ARR_ARR(%0,%1,%2,%3,%4,%5)%8$        %8$%0 %1%2[%3][%4][%5]
#define REBUILD_ARR_ARR_ARR_ARR(%0,%1,%2,%3,%4,%5,%6)%8$ %8$%0 %1%2[%3][%4][%5][%6]
```

If there is a `const `, it will appear.  If there isn't it won't.  If there is a
tag it will appear.  If there isn't it won't.  Further, the `const ` parameter
actually includes its own space for efficiency reasons, so this works too:

```pawn
#define REBUILD_NUM(%0,%1,%2)%8$                         %8$%0%1%2
#define REBUILD_NUM_DEF(%0,%1,%2,%4)%8$                  %8$%0%1%2=%4
#define REBUILD_REF(,%1,%2)%8$                           %8$&%1%2
#define REBUILD_REF_DEF(,%1,%2,%4)%8$                    %8$&%1%2=%4
#define REBUILD_EXT(,%1,)%8$                             %8$%1...
#define REBUILD_STR(%0,,%2,%3)%8$                        %8$%0%2[%3]
#define REBUILD_STR_DEF(%0,,%2,%3,%4)%8$                 %8$%0%2[%3]=%4
#define REBUILD_ARR(%0,%1,%2,%3)%8$                      %8$%0%1%2[%3]
#define REBUILD_ARR_ARR(%0,%1,%2,%3,%4)%8$               %8$%0%1%2[%3][%4]
#define REBUILD_ARR_ARR_ARR(%0,%1,%2,%3,%4,%5)%8$        %8$%0%1%2[%3][%4][%5]
#define REBUILD_ARR_ARR_ARR_ARR(%0,%1,%2,%3,%4,%5,%6)%8$ %8$%0%1%2[%3][%4][%5][%6]
```

### Output Everything

That code will correctly render each individual parameter, but will not
correctly reform a full function call.  There are no commas and, because there
is no matching against everything already output, this will reverse the
parameters (which is a cool trick, but not what we want here).  The larger
version would look like the following, using a second `$` to denote the current
end of the parameter list:

```pawn
#define REBUILD:%0(%1) FUNC_PARSER(REBUILD,ARR_MUL_TAG_CST:NUM_TAG_CST_DEF:REF_TAG_DEF:EXT_TAG:STR_CST_DEF:)(%0(%1))%0($)

#define REBUILD_NUM(%0,%1,%2)%8$%7(%9$)                         %8$%7(%9, %0 %1%2                $)
#define REBUILD_NUM_DEF(%0,%1,%2,%4)%8$%7(%9$)                  %8$%7(%9, %0 %1%2     = %4       $)
#define REBUILD_REF(,%1,%2)%8$%7(%9$)                           %8$%7(%9,  & %1%2                $)
#define REBUILD_REF_DEF(,%1,%2,%4)%8$%7(%9$)                    %8$%7(%9,  & %1%2     = %4       $)
#define REBUILD_EXT(,%1,)%8$%7(%9$)                             %8$%7(%9,    %1...               $)
#define REBUILD_STR(%0,,%2,%3)%8$%7(%9$)                        %8$%7(%9, %0   %2[%3]            $)
#define REBUILD_STR_DEF(%0,,%2,%3,%4)%8$%7(%9$)                 %8$%7(%9, %0   %2[%3] = %4       $)
#define REBUILD_ARR(%0,%1,%2,%3)%8$%7(%9$)                      %8$%7(%9, %0 %1%2[%3]            $)
#define REBUILD_ARR_ARR(%0,%1,%2,%3,%4)%8$%7(%9$)               %8$%7(%9, %0 %1%2[%3][%4]        $)
#define REBUILD_ARR_ARR_ARR(%0,%1,%2,%3,%4,%5)%8$%7(%9$)        %8$%7(%9, %0 %1%2[%3][%4][%5]    $)
#define REBUILD_ARR_ARR_ARR_ARR(%0,%1,%2,%3,%4,%5,%6)%8$%7(%9$) %8$%7(%9, %0 %1%2[%3][%4][%5][%6]$)

#define REBUILD_END(%9)%8$%7(,%9$) %8$%7(%9)
#define REBUILD_NUL(%9)%8$%7($)    %8$%7()
```

Here the two ending macros come in useful.  Before they trigger the first
parameter will have a leading comma, and the last parameter will have a trailing
dollar.  They remove those match artifacts.

## Example 6 - Remotes With Alternate Return Types

### No Return

In this library and others (YSI) "no return" uses the C naming of `void` when
the code from returns and no returns is different.



## API

The `FUNC_PARSER` macro is the main API entry-point, and generates the macros to
do the parsing.  The reason it is done this way is that things like
`TAG_NUM_DEFAULT:` are pretty descriptive tags - that (relatively) clearly
indicates that you want the parser to detect numbers (regular variables, but
`VAR` could be confused for both "variable" and "vararg", hence it is avoided).
However, that tag is very long, this tag-macro based parsing method leaves a lot
of unmatched macros behind, and it is recursive.  Long macros with short line-
length limits do not play well together.

The include is quite long, but it doesn't need to be.  The vast majority of the
code is to convert something like this:

```pawn
FUNC_PARSER(REBUILD,ARRAY_TAG_CONST_MULTI:NUMBER_CONST_TAG_DEFAULT:TAG_REFERENCE_DEFAULT:TAG_VARARG:DEFAULT_STRING_CONST)(func(params))
```

In to this:

```pawn
__:z@:l@:w@:x@<c@:b@:>a@:x@<f@:c@:t@:>v@:x@<t@:>r@:x@<t@:d@:>o@:x@<c@:t@:d@:>(REBUILD)func(||||||params,)$
```

I.e. just provides a nice API for using the parser.  The actual complex part is
done by these few macros.  Feel free to take just these if you want and derive
your own input to the magical `__:` macro:

```pawn
#define void:
#define string:
#define u@$ 0]);
#define __:%0$ u@(u[_:%0 u@$
#define c@:%8(%0,%1,%9const%2) %8(%0const ,%1,%9%2)
#define t@:%8(%0,%1,%9:%2) %8(%0,%1%9:,%2)
#define d@:%8(%0,%1,%2=%9) %8_DEF(%0,%1,%2,%9)
#define b@:%8(%0,%1,%2=%9,%4) %8_DEF(%0,%1,%2,%4,%9)
#define f@:%8(%0,%1,%2[%9]%6,%3) f@:%8_ARR(%0,%1,%2%6,%3,%9)
#define x@<%9>
#define w@:x@<%8>%9.%0(%5)(%4|||%6string:%1[%2]%9|||%7)%3$ %8%5_STR(,,%6%1%9,%2)%0(%5)(||||||%7)%3$
#define a@:x@<%8>%9.%0(%5)(%4|||%1[%2]%9|||%7)%3$ %8%5_ARR(,,%1%9,%2)%0(%5)(||||||%7)%3$
#define r@:x@<%8>%9.%0(%5)(%4|||%1&%2|||%7)%3$ %8%5_REF(,,%1%2)%0(%5)(||||||%7)%3$
#define v@:x@<%8>%9.%0(%5)(%4|||%1...%2|||%7)%3$ %8%5_EXT(,,%1%2)%0(%5)(||||||%7)%3$
#define o@:x@<%8>%9.%0(%5)(%4|||%1|||%7)%3$ %8%5_NUM(,,%1)%0(%5)(||||||%7)%3$
#define z@:%0(%5)(||||||,%7)%3$ %5_NUL()%7%3$
#define n@:%0(%5)(%4|||%6|||%7)%3$ %5_END()%7%3$
#define m@:n@:%0(%5)(%4|||%6|||%7,%8)%3$ %0.m@:n@:%0(%5)(%4%6|||%7|||%8)%3$
```

But don't be upset if the underlying macros change...

