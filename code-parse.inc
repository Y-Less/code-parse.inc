/*
	Version: MPL 1.1
	
	The contents of this file are subject to the Mozilla Public License Version 
	1.1 (the "License"); you may not use this file except in compliance with 
	the License. You may obtain a copy of the License at 
	http://www.mozilla.org/MPL/
	
	Software distributed under the License is distributed on an "AS IS" basis,
	WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
	for the specific language governing rights and limitations under the
	License.
	
	The Original Code is the code-parse.inc code analyser.
	
	The Initial Developer of the Original Code is Alex "Y_Less" Cole.
	Portions created by the Initial Developer are Copyright (C) 2016
	the Initial Developer. All Rights Reserved.
*/

// This include provides a handy set of macros for advanced compile-time code
// analysis and generation.  With this, one can do things like analyse the
// parameter types for a function, auto-generate call specifiers, and otherwise
// create code-dependent macros.

#define @END%0$ %0
#define @OB (
#define @CB )
#define @CM ,
#define @DL $

// Remove this tag from any final output after all other parsing is done.
#define string:
#define void:

// A very generic entry point.  This can be used by any other system to put
// arbitrary macros at the top level of the file without compiler errors.
#define u@$ 0]);
#define __:%0$ u@(u[_:%0 u@$

// This is the "global" entry point.  It takes a list of parser types, followed
// by a custom macro name prefix for the user code, followed by the parameters
// to parse.  We always have "o@:" on the end to detect remaining parameters
// (this will detect any type not covered by others).
#define FUNC_PARSER(%0,%1)(%3(%4)) h@%1(,,,,,o@:x@[])(%0)%3(%4,)

// Expression entry point.
#define e@

// Statement entry point.
#define s@

// Modifiers for parameter types.  These detect optional extra bits such as
// "const" and tags.

// Detect "const".
#define c@:%8(%0,%1,%9const%2) %8(%0const ,%1,%9%2)

// Detect tags.
#define t@:%8(%0,%1,%9:%2) %8(%0,%1%9:,%2)

// Detect defaults.
#define d@:%8(%0,%1,%2=%9) %8_DEF(%0,%1,%2,%9)
#define b@:%8(%0,%1,%2=%9,%4) %8_DEF(%0,%1,%2,%4,%9)

// Detect recursive array sizes (multi-dimensional arrays).
#define f@:%8(%0,%1,%2[%9]%6,%3) f@:%8_ARR(%0,%1,%2%6,%3,%9)

// Remove specialisers after we are done with this parameter type.
#define x@[%9]

// Redistribute the new parsers to after the current parameter.
#define LENGTH_REQUIRED{%9}%0(%1)%2(%3)%4(|||%6,%7) %0(%1)j@:q@:%9.%2(%3)%4(%6|||%7)

// Detect "string:" (words).
#define w@:x@[%8]%9.%0(%5)%3(%6string:%1[%2]%9|||%7) %8%5_STR(,,%6%1%9,%2)%0(%5)%3(|||%7)

// Detect "[]".
#define a@:x@[%8]%9.%0(%5)%3(%1[%2]%9|||%7) %8%5_ARR(,,%1%9,%2)%0(%5)%3(|||%7)

#define j@:%0(%5)%3(%1[%2]%9|||%7) LENGTH_REQUIRED()
#define q@:%0(%5)%3(%1...%2|||%7)  LENGTH_REQUIRED()

// Detect "&".
#define r@:x@[%8]%9.%0(%5)%3(%1&%2|||%7) %8%5_REF(,,%1%2)%0(%5)%3(|||%7)

// Detect varargs.
#define v@:x@[%8]%9.%0(%5)%3(%1...%2|||%7) %8%5_EXT(,,%1%2)%0(%5)%3(|||%7)

// Detect others.
#define o@:x@[%8]%9.%0(%5)%3(%1|||%7) %8%5_NUM(,,%1)%0(%5)%3(|||%7)

// Detect none.
#define z@:%0[%9]%0(%5)%3(|||,%7) %9%5_NUL(%3)%7

// Detect out of parameters.
#define n@[%9]%0(%5)%3(%6|||%7) %9%5_END(%3)%7

// Not none, thus many parameters.  Used to recurse through multiple parameters.
#define m@:n@[%9]%0(%5)%3(%6|||%7,%8) %0.m@:n@[%9]%0(%5)%3(%7|||%8)

// Detect possible return types.
#define g@:%9-%5(%9void:%3) %5_VOD(%3)
#define k@:%9-%5(%9string:%3) %5_STR(%3)
#define l@:%9-%5(%9:%3) %5_TAG(%9:,%3)
#define p@-

// We need the macros in a very specific order, because some will match others
// as well.  For example, arrays will match strings but not vice-versa, so we
// need to check for strings first.  Additionally, requesting scans for arrays
// with tags and arrays without tags (for example) makes no sense - just use the
// former.  For this reason we compress all the options by joining them in a
// list first then expanding that list:
// 
//   %0 - String detection options.     string:a[]
//   %1 - Array detection options.      a[]
//   %2 - Reference detection options.  &a
//   %3 - Vararg detection options.     ...     
//   %4 - Other detection options.      a
// 
// Obviously "other" comes last as it will match literally anything else.  The
// string and array options must be relatively ordered, but their order relative
// to "&" and "..." is arbitrary.  Once all the type options are collected, the
// "h@WHATEVER" macros will stop triggering and the "h@()" macro will trigger
// instead to combine all the tag macros.
#define h@(%9,%0,%1,%2,%3,%4)(%5)%6(%7) __:z@:m@:n@[%9]%0%1%2%3%4(%5)%6(|||%7)$

// Return types.
#define h@RET_VOD:%8(%9,%0,%1,%2,%3,%4)         h@%8(g@:p@-,%0,%1,%2,%3,%4)       // void:
#define h@RET_VOD_STR:%8(%9,%0,%1,%2,%3,%4)     h@%8(g@:k@:p@-,%0,%1,%2,%3,%4)    // void: string:
#define h@RET_STR_VOD                       h@RET_VOD_STR
#define h@RET_VOD_TAG:%8(%9,%0,%1,%2,%3,%4)     h@%8(g@:l@:p@-,%0,%1,%2,%3,%4)    // void: ???:
#define h@RET_TAG_VOD                       h@RET_VOD_TAG
#define h@RET_VOD_STR_TAG:%8(%9,%0,%1,%2,%3,%4) h@%8(g@:k@:l@:p@-,%0,%1,%2,%3,%4) // void: string: ???:
#define h@RET_STR_VOD_TAG                   h@RET_VOD_STR_TAG
#define h@RET_VOD_TAG_STR                   h@RET_VOD_STR_TAG
#define h@RET_STR_TAG_VOD                   h@RET_VOD_STR_TAG
#define h@RET_TAG_VOD_STR                   h@RET_VOD_STR_TAG
#define h@RET_TAG_STR_VOD                   h@RET_VOD_STR_TAG
#define h@RET_TAG:%8(%9,%0,%1,%2,%3,%4)         h@%8(l@:p@-,%0,%1,%2,%3,%4)       // ???:
#define h@RET_STR:%8(%9,%0,%1,%2,%3,%4)         h@%8(k@:p@-,%0,%1,%2,%3,%4)       // string:
#define h@RET_STR_TAG:%8(%9,%0,%1,%2,%3,%4)     h@%8(k@:l@:p@-,%0,%1,%2,%3,%4)    // string: ???:
#define h@RET_TAG_STR                       h@RET_STR_TAG

#define h@RET_VOID            h@RET_VOD
#define h@RET_VOID_STRING     h@RET_VOD_STR
#define h@RET_STRING_VOID     h@RET_VOD_STR
#define h@RET_VOID_TAG        h@RET_VOD_TAG
#define h@RET_TAG_VOID        h@RET_VOD_TAG
#define h@RET_VOID_STRING_TAG h@RET_VOD_STR_TAG
#define h@RET_STRING_VOID_TAG h@RET_VOD_STR_TAG
#define h@RET_VOID_TAG_STRING h@RET_VOD_STR_TAG
#define h@RET_STRING_TAG_VOID h@RET_VOD_STR_TAG
#define h@RET_TAG_VOID_STRING h@RET_VOD_STR_TAG
#define h@RET_TAG_STRING_VOID h@RET_VOD_STR_TAG
#define h@RET_STRING          h@RET_STR
#define h@RET_STRING_TAG      h@RET_STR_TAG
#define h@RET_TAG_STRING      h@RET_STR_TAG

// Enforce array lengths.
// #define h@LEN:%8(%9,%0,a@:x@[%1],%2,%3,o@:x@[%4]) h@(%9,%0,q@:x@[%1j@{%4}],%2,%3,o@:x@[%4])
#define h@LEN:%8(%9,%0,a@:x@[%1],%2,%3,%4) h@(%9,%0,a@:x@[%1LENGTH_REQUIRED{%2%4}],%2,%3,%4)
#define h@LENGTH h@LEN

// All the parameter types that can be detected (hopefully all).  "tag" macros
// will also match multi-tag types such as "{tag, _}:..."
#define h@ARR:%8(%9,%0,%1,%2,%3,%4)             h@%8(%9,%0,a@:x@[],%2,%3,%4)          // a[]
#define h@NUM_TAG:%8(%9,%0,%1,%2,%3,%4)         h@%8(%9,%0,%1,%2,%3,o@:x@[t@:])       // tag:a
#define h@TAG_NUM                           h@NUM_TAG
#define h@STR:%8(%9,%0,%1,%2,%3,%4)             h@%8(%9,w@:x@[],%1,%2,%3,%4)          // string:a[]
#define h@STR_DEF:%8(%9,%0,%1,%2,%3,%4)         h@%8(%9,w@:x@[b@:],%1,%2,%3,%4)       // string:a[] = "hi"
#define h@DEF_STR                           h@STR_DEF
#define h@NUM:%8(%9,%0,%1,%2,%3,%4)             h@%8(%9,%0,%1,%2,%3,o@:x@[])          // a
#define h@ARR_TAG:%8(%9,%0,%1,%2,%3,%4)         h@%8(%9,%0,a@:x@[t@:],%2,%3,%4)       // tag:a[]
#define h@TAG_ARR                           h@ARR_TAG
#define h@NUM_TAG_DEF:%8(%9,%0,%1,%2,%3,%4)     h@%8(%9,%0,%1,%2,%3,o@:x@[t@:d@:])    // tag:a = 5
#define h@NUM_DEF_TAG                       h@NUM_TAG_DEF
#define h@TAG_NUM_DEF                       h@NUM_TAG_DEF
#define h@DEF_NUM_TAG                       h@NUM_TAG_DEF
#define h@TAG_DEF_NUM                       h@NUM_TAG_DEF
#define h@DEF_TAG_NUM                       h@NUM_TAG_DEF
#define h@NUM_DEF:%8(%9,%0,%1,%2,%3,%4)         h@%8(%9,%0,%1,%2,%3,o@:x@[d@:])       // a = 5
#define h@DEF_NUM                           h@NUM_DEF
#define h@REF:%8(%9,%0,%1,%2,%3,%4)             h@%8(%9,%0,%1,r@:x@[],%3,%4)          // &a
#define h@REF_TAG:%8(%9,%0,%1,%2,%3,%4)         h@%8(%9,%0,%1,r@:x@[t@:],%3,%4)       // &tag:a
#define h@TAG_REF                           h@REF_TAG
#define h@REF_DEF:%8(%9,%0,%1,%2,%3,%4)         h@%8(%9,%0,%1,r@:x@[d@:],%3,%4)       // &a = 5
#define h@DEF_REF                           h@REF_DEF
#define h@REF_TAG_DEF:%8(%9,%0,%1,%2,%3,%4)     h@%8(%9,%0,%1,r@:x@[t@:d@:],%3,%4)    // &tag:a = 5
#define h@REF_DEF_TAG                       h@REF_TAG_DEF
#define h@TAG_REF_DEF                       h@REF_TAG_DEF
#define h@TAG_DEF_REF                       h@REF_TAG_DEF
#define h@DEF_REF_TAG                       h@REF_TAG_DEF
#define h@DEF_TAG_REF                       h@REF_TAG_DEF
#define h@EXT:%8(%9,%0,%1,%2,%3,%4)             h@%8(%9,%0,%1,%2,v@:x@[],%4)          // ...
#define h@EXT_TAG:%8(%9,%0,%1,%2,%3,%4)         h@%8(%9,%0,%1,%2,v@:x@[t@:],%4)       // tag:...
#define h@TAG_EXT                           h@EXT_TAG
#define h@ARR_CST:%8(%9,%0,%1,%2,%3,%4)         h@%8(%9,%0,a@:x@[c@:],%2,%3,%4)       // const a[]
#define h@CST_ARR                           h@ARR_CST
#define h@NUM_CST_TAG:%8(%9,%0,%1,%2,%3,%4)     h@%8(%9,%0,%1,%2,%3,o@:x@[c@:t@:])    // const tag:a
#define h@NUM_TAG_CST                       h@NUM_CST_TAG
#define h@TAG_NUM_CST                       h@NUM_CST_TAG
#define h@TAG_CST_NUM                       h@NUM_CST_TAG
#define h@CST_NUM_TAG                       h@NUM_CST_TAG
#define h@CST_TAG_NUM                       h@NUM_CST_TAG
#define h@STR_CST:%8(%9,%0,%1,%2,%3,%4)         h@%8(%9,w@:x@[c@:],%1,%2,%3,%4)       // const string:a[]
#define h@CST_STR                           h@STR_CST
#define h@STR_CST_DEF:%8(%9,%0,%1,%2,%3,%4)     h@%8(%9,w@:x@[c@:b@:],%1,%2,%3,%4)    // const string:a[] = "hi"
#define h@CST_DEF_STR                       h@STR_CST_DEF
#define h@CST_STR_DEF                       h@STR_CST_DEF
#define h@DEF_CST_STR                       h@STR_CST_DEF
#define h@DEF_STR_CST                       h@STR_CST_DEF
#define h@STR_DEF_CST                       h@STR_CST_DEF
#define h@NUM_CST:%8(%9,%0,%1,%2,%3,%4)         h@%8(%9,%0,%1,%2,%3,o@:x@[c@:])       // const a
#define h@CST_NUM                           h@NUM_CST
#define h@ARR_CST_TAG:%8(%9,%0,%1,%2,%3,%4)     h@%8(%9,%0,a@:x@[c@:t@:],%2,%3,%4)    // const tag:a[]
#define h@CST_TAG_ARR                       h@ARR_CST_TAG
#define h@CST_ARR_TAG                       h@ARR_CST_TAG
#define h@ARR_TAG_CST                       h@ARR_CST_TAG
#define h@TAG_CST_ARR                       h@ARR_CST_TAG
#define h@TAG_ARR_CST                       h@ARR_CST_TAG
#define h@NUM_CST_TAG_DEF:%8(%9,%0,%1,%2,%3,%4) h@%8(%9,%0,%1,%2,%3,o@:x@[c@:t@:d@:]) // const tag:a = 5
#define h@NUM_CST_DEF_TAG                   h@NUM_CST_TAG_DEF
#define h@NUM_DEF_CST_TAG                   h@NUM_CST_TAG_DEF
#define h@NUM_DEF_TAG_CST                   h@NUM_CST_TAG_DEF
#define h@NUM_TAG_CST_DEF                   h@NUM_CST_TAG_DEF
#define h@NUM_TAG_DEF_CST                   h@NUM_CST_TAG_DEF
#define h@CST_NUM_TAG_DEF                   h@NUM_CST_TAG_DEF
#define h@CST_NUM_DEF_TAG                   h@NUM_CST_TAG_DEF
#define h@DEF_NUM_CST_TAG                   h@NUM_CST_TAG_DEF
#define h@DEF_NUM_TAG_CST                   h@NUM_CST_TAG_DEF
#define h@TAG_NUM_CST_DEF                   h@NUM_CST_TAG_DEF
#define h@TAG_NUM_DEF_CST                   h@NUM_CST_TAG_DEF
#define h@CST_TAG_NUM_DEF                   h@NUM_CST_TAG_DEF
#define h@CST_DEF_NUM_TAG                   h@NUM_CST_TAG_DEF
#define h@DEF_CST_NUM_TAG                   h@NUM_CST_TAG_DEF
#define h@DEF_TAG_NUM_CST                   h@NUM_CST_TAG_DEF
#define h@TAG_CST_NUM_DEF                   h@NUM_CST_TAG_DEF
#define h@TAG_DEF_NUM_CST                   h@NUM_CST_TAG_DEF
#define h@CST_TAG_DEF_NUM                   h@NUM_CST_TAG_DEF
#define h@CST_DEF_TAG_NUM                   h@NUM_CST_TAG_DEF
#define h@DEF_CST_TAG_NUM                   h@NUM_CST_TAG_DEF
#define h@DEF_TAG_CST_NUM                   h@NUM_CST_TAG_DEF
#define h@TAG_CST_DEF_NUM                   h@NUM_CST_TAG_DEF
#define h@TAG_DEF_CST_NUM                   h@NUM_CST_TAG_DEF
#define h@NUM_CST_DEF:%8(%9,%0,%1,%2,%3,%4)     h@%8(%9,%0,%1,%2,%3,o@:x@[c@:d@:])    // const a = 5
#define h@CST_NUM_DEF                       h@NUM_CST_DEF
#define h@CST_DEF_NUM                       h@NUM_CST_DEF
#define h@DEF_CST_NUM                       h@NUM_CST_DEF
#define h@DEF_NUM_CST                       h@NUM_CST_DEF
#define h@NUM_DEF_CST                       h@NUM_CST_DEF

// Less short names of macros.
#define h@ARRAY                    h@ARR
#define h@NUMBER_TAG               h@NUM_TAG
#define h@TAG_NUMBER               h@NUM_TAG
#define h@STRING                   h@STR
#define h@DEFAULT_STRING           h@STR_DEF
#define h@STRING_DEFAULT           h@STR_DEF
#define h@NUMBER                   h@NUM
#define h@TAG_ARRAY                h@ARR_TAG
#define h@ARRAY_TAG                h@ARR_TAG
#define h@NUMBER_TAG_DEFAULT       h@NUM_TAG_DEF
#define h@NUMBER_DEFAULT_TAG       h@NUM_TAG_DEF
#define h@TAG_NUMBER_DEFAULT       h@NUM_TAG_DEF
#define h@DEFAULT_NUMBER_TAG       h@NUM_TAG_DEF
#define h@TAG_DEFAULT_NUMBER       h@NUM_TAG_DEF
#define h@DEFAULT_TAG_NUMBER       h@NUM_TAG_DEF
#define h@NUMBER_DEFAULT           h@NUM_DEF
#define h@DEFAULT_NUMBER           h@NUM_DEF
#define h@REFERENCE                h@REF
#define h@TAG_REFERENCE            h@REF_TAG
#define h@REFERENCE_TAG            h@REF_TAG
#define h@DEFAULT_REFERENCE        h@REF_DEF
#define h@REFERENCE_DEFAULT        h@REF_DEF
#define h@TAG_REFERENCE_DEFAULT    h@REF_TAG_DEF
#define h@TAG_DEFAULT_REFERENCE    h@REF_TAG_DEF
#define h@REFERENCE_TAG_DEFAULT    h@REF_TAG_DEF
#define h@REFERENCE_DEFAULT_TAG    h@REF_TAG_DEF
#define h@DEFAULT_REFERENCE_TAG    h@REF_TAG_DEF
#define h@DEFAULT_TAG_REFERENCE    h@REF_TAG_DEF
#define h@VARARG                   h@EXT
#define h@VARARG_TAG               h@EXT_TAG
#define h@TAG_VARARG               h@EXT_TAG
#define h@CONST_ARRAY              h@ARR_CST
#define h@ARRAY_CONST              h@ARR_CST
#define h@NUMBER_CONST_TAG         h@NUM_CST_TAG
#define h@NUMBER_TAG_CONST         h@NUM_CST_TAG
#define h@CONST_NUMBER_TAG         h@NUM_CST_TAG
#define h@TAG_NUMBER_CONST         h@NUM_CST_TAG
#define h@CONST_TAG_NUMBER         h@NUM_CST_TAG
#define h@TAG_CONST_NUMBER         h@NUM_CST_TAG
#define h@CONST_STRING             h@STR_CST
#define h@STRING_CONST             h@STR_CST
#define h@CONST_DEFAULT_STRING     h@STR_CST_DEF
#define h@CONST_STRING_DEFAULT     h@STR_CST_DEF
#define h@DEFAULT_CONST_STRING     h@STR_CST_DEF
#define h@DEFAULT_STRING_CONST     h@STR_CST_DEF
#define h@STRING_CONST_DEFAULT     h@STR_CST_DEF
#define h@STRING_DEFAULT_CONST     h@STR_CST_DEF
#define h@CONST_NUMBER             h@NUM_CST
#define h@NUMBER_CONST             h@NUM_CST
#define h@CONST_TAG_ARRAY          h@ARR_CST_TAG
#define h@CONST_ARRAY_TAG          h@ARR_CST_TAG
#define h@ARRAY_CONST_TAG          h@ARR_CST_TAG
#define h@ARRAY_TAG_CONST          h@ARR_CST_TAG
#define h@TAG_CONST_ARRAY          h@ARR_CST_TAG
#define h@TAG_ARRAY_CONST          h@ARR_CST_TAG
#define h@NUMBER_CONST_TAG_DEFAULT h@NUM_CST_TAG_DEF
#define h@NUMBER_CONST_DEFAULT_TAG h@NUM_CST_TAG_DEF
#define h@NUMBER_DEFAULT_CONST_TAG h@NUM_CST_TAG_DEF
#define h@NUMBER_DEFAULT_TAG_CONST h@NUM_CST_TAG_DEF
#define h@NUMBER_TAG_CONST_DEFAULT h@NUM_CST_TAG_DEF
#define h@NUMBER_TAG_DEFAULT_CONST h@NUM_CST_TAG_DEF
#define h@CONST_NUMBER_TAG_DEFAULT h@NUM_CST_TAG_DEF
#define h@CONST_NUMBER_DEFAULT_TAG h@NUM_CST_TAG_DEF
#define h@DEFAULT_NUMBER_CONST_TAG h@NUM_CST_TAG_DEF
#define h@DEFAULT_NUMBER_TAG_CONST h@NUM_CST_TAG_DEF
#define h@TAG_NUMBER_CONST_DEFAULT h@NUM_CST_TAG_DEF
#define h@TAG_NUMBER_DEFAULT_CONST h@NUM_CST_TAG_DEF
#define h@CONST_TAG_NUMBER_DEFAULT h@NUM_CST_TAG_DEF
#define h@CONST_DEFAULT_NUMBER_TAG h@NUM_CST_TAG_DEF
#define h@DEFAULT_CONST_NUMBER_TAG h@NUM_CST_TAG_DEF
#define h@DEFAULT_TAG_NUMBER_CONST h@NUM_CST_TAG_DEF
#define h@TAG_CONST_NUMBER_DEFAULT h@NUM_CST_TAG_DEF
#define h@TAG_DEFAULT_NUMBER_CONST h@NUM_CST_TAG_DEF
#define h@CONST_TAG_DEFAULT_NUMBER h@NUM_CST_TAG_DEF
#define h@CONST_DEFAULT_TAG_NUMBER h@NUM_CST_TAG_DEF
#define h@DEFAULT_CONST_TAG_NUMBER h@NUM_CST_TAG_DEF
#define h@DEFAULT_TAG_CONST_NUMBER h@NUM_CST_TAG_DEF
#define h@TAG_CONST_DEFAULT_NUMBER h@NUM_CST_TAG_DEF
#define h@TAG_DEFAULT_CONST_NUMBER h@NUM_CST_TAG_DEF
#define h@CONST_NUMBER_DEFAULT     h@NUM_CST_DEF
#define h@CONST_DEFAULT_NUMBER     h@NUM_CST_DEF
#define h@DEFAULT_CONST_NUMBER     h@NUM_CST_DEF
#define h@DEFAULT_NUMBER_CONST     h@NUM_CST_DEF
#define h@NUMBER_CONST_DEFAULT     h@NUM_CST_DEF
#define h@NUMBER_DEFAULT_CONST     h@NUM_CST_DEF

// Multi-dimensional arrays.
#define h@ARR_MUL:%8(%9,%0,%1,%2,%3,%4)         h@%8(%9,%0,a@:x@[f@:],%2,%3,%4)       // a[][]
#define h@MUL_ARR                           h@ARR_MUL
#define h@ARR_TAG_MUL:%8(%9,%0,%1,%2,%3,%4)     h@%8(%9,%0,a@:x@[f@:t@:],%2,%3,%4)    // tag:a[][]
#define h@TAG_MUL_ARR                       h@ARR_TAG_MUL
#define h@TAG_ARR_MUL                       h@ARR_TAG_MUL
#define h@ARR_MUL_TAG                       h@ARR_TAG_MUL
#define h@MUL_ARR_TAG                       h@ARR_TAG_MUL
#define h@MUL_TAG_ARR                       h@ARR_TAG_MUL
#define h@ARR_CST_MUL:%8(%9,%0,%1,%2,%3,%4)     h@%8(%9,%0,a@:x@[f@:c@:],%2,%3,%4)    // const a[][]
#define h@CST_MUL_ARR                       h@ARR_CST_MUL
#define h@CST_ARR_MUL                       h@ARR_CST_MUL
#define h@ARR_MUL_CST                       h@ARR_CST_MUL
#define h@MUL_CST_ARR                       h@ARR_CST_MUL
#define h@MUL_ARR_CST                       h@ARR_CST_MUL
#define h@ARR_CST_TAG_MUL:%8(%9,%0,%1,%2,%3,%4) h@%8(%9,%0,a@:x@[f@:c@:t@:],%2,%3,%4) // const tag:a[][]
#define h@CST_TAG_ARR_MUL                   h@ARR_CST_TAG_MUL
#define h@CST_TAG_MUL_ARR                   h@ARR_CST_TAG_MUL
#define h@CST_ARR_TAG_MUL                   h@ARR_CST_TAG_MUL
#define h@CST_ARR_MUL_TAG                   h@ARR_CST_TAG_MUL
#define h@CST_MUL_ARR_TAG                   h@ARR_CST_TAG_MUL
#define h@CST_MUL_TAG_ARR                   h@ARR_CST_TAG_MUL
#define h@TAG_CST_ARR_MUL                   h@ARR_CST_TAG_MUL
#define h@TAG_CST_MUL_ARR                   h@ARR_CST_TAG_MUL
#define h@ARR_CST_MUL_TAG                   h@ARR_CST_TAG_MUL
#define h@MUL_CST_ARR_TAG                   h@ARR_CST_TAG_MUL
#define h@MUL_CST_TAG_ARR                   h@ARR_CST_TAG_MUL
#define h@TAG_ARR_CST_MUL                   h@ARR_CST_TAG_MUL
#define h@TAG_MUL_CST_ARR                   h@ARR_CST_TAG_MUL
#define h@ARR_TAG_CST_MUL                   h@ARR_CST_TAG_MUL
#define h@ARR_MUL_CST_TAG                   h@ARR_CST_TAG_MUL
#define h@MUL_ARR_CST_TAG                   h@ARR_CST_TAG_MUL
#define h@MUL_TAG_CST_ARR                   h@ARR_CST_TAG_MUL
#define h@TAG_ARR_MUL_CST                   h@ARR_CST_TAG_MUL
#define h@TAG_MUL_ARR_CST                   h@ARR_CST_TAG_MUL
#define h@ARR_TAG_MUL_CST                   h@ARR_CST_TAG_MUL
#define h@ARR_MUL_TAG_CST                   h@ARR_CST_TAG_MUL
#define h@MUL_ARR_TAG_CST                   h@ARR_CST_TAG_MUL
#define h@MUL_TAG_ARR_CST                   h@ARR_CST_TAG_MUL
#define h@ARRAY_MULTI           h@ARR_MUL
#define h@MULTI_ARRAY           h@ARR_MUL
#define h@TAG_ARRAY_MULTI       h@ARR_TAG_MUL
#define h@TAG_MULTI_ARRAY       h@ARR_TAG_MUL
#define h@ARRAY_TAG_MULTI       h@ARR_TAG_MUL
#define h@ARRAY_MULTI_TAG       h@ARR_TAG_MUL
#define h@MULTI_ARRAY_TAG       h@ARR_TAG_MUL
#define h@MULTI_TAG_ARRAY       h@ARR_TAG_MUL
#define h@CONST_MULTI_ARRAY     h@ARR_CST_MUL
#define h@CONST_ARRAY_MULTI     h@ARR_CST_MUL
#define h@ARRAY_CONST_MULTI     h@ARR_CST_MUL
#define h@ARRAY_MULTI_CONST     h@ARR_CST_MUL
#define h@MULTI_CONST_ARRAY     h@ARR_CST_MUL
#define h@MULTI_ARRAY_CONST     h@ARR_CST_MUL
#define h@CONST_TAG_ARRAY_MULTI h@ARR_CST_TAG_MUL
#define h@CONST_TAG_MULTI_ARRAY h@ARR_CST_TAG_MUL
#define h@CONST_ARRAY_TAG_MULTI h@ARR_CST_TAG_MUL
#define h@CONST_ARRAY_MULTI_TAG h@ARR_CST_TAG_MUL
#define h@CONST_MULTI_ARRAY_TAG h@ARR_CST_TAG_MUL
#define h@CONST_MULTI_TAG_ARRAY h@ARR_CST_TAG_MUL
#define h@TAG_CONST_ARRAY_MULTI h@ARR_CST_TAG_MUL
#define h@TAG_CONST_MULTI_ARRAY h@ARR_CST_TAG_MUL
#define h@ARRAY_CONST_TAG_MULTI h@ARR_CST_TAG_MUL
#define h@ARRAY_CONST_MULTI_TAG h@ARR_CST_TAG_MUL
#define h@MULTI_CONST_ARRAY_TAG h@ARR_CST_TAG_MUL
#define h@MULTI_CONST_TAG_ARRAY h@ARR_CST_TAG_MUL
#define h@TAG_ARRAY_CONST_MULTI h@ARR_CST_TAG_MUL
#define h@TAG_MULTI_CONST_ARRAY h@ARR_CST_TAG_MUL
#define h@ARRAY_TAG_CONST_MULTI h@ARR_CST_TAG_MUL
#define h@ARRAY_MULTI_CONST_TAG h@ARR_CST_TAG_MUL
#define h@MULTI_ARRAY_CONST_TAG h@ARR_CST_TAG_MUL
#define h@MULTI_TAG_CONST_ARRAY h@ARR_CST_TAG_MUL
#define h@TAG_ARRAY_MULTI_CONST h@ARR_CST_TAG_MUL
#define h@TAG_MULTI_ARRAY_CONST h@ARR_CST_TAG_MUL
#define h@ARRAY_TAG_MULTI_CONST h@ARR_CST_TAG_MUL
#define h@ARRAY_MULTI_TAG_CONST h@ARR_CST_TAG_MUL
#define h@MULTI_ARRAY_TAG_CONST h@ARR_CST_TAG_MUL
#define h@MULTI_TAG_ARRAY_CONST h@ARR_CST_TAG_MUL

