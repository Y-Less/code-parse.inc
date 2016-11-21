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
#define FUNC_PARSER(%0,%1)(%3(%4)) PARSE@%1(p@--,,,,,o@:x@[])(%0)%3(%4,)

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
#define z@:%0[%9]%0(%5)%3(|||,%7) %9%5_NUL(%3)n@DROP[%5]%7

// Detect out of parameters.
#define n@[%9]%0(%5)%3(%6|||%7) %9%5_END(%3)n@DROP[%5]%7
#define n@DROP[%5]

// Not none, thus many parameters.  Used to recurse through multiple parameters.
#define m@:n@[%9]%0(%5)%3(%6|||%7,%8) %0.m@:n@[%9]%0(%5)%3(%7|||%8)

// Detect possible return types.
#define g@:%9-%6-%5(%9void:%3) %6%5_VOD(%3)
#define k@:%9-%6-%5(%9string:%3) %6%5_STR(%3)
#define l@:%9-%6-%5(%9:%3) %6PARSE_WILL_BE(%9)%5_TAG<:,%3>
#define p@-%6-%5(%3) %6%5(%3)
#define PARSE_WILL_BE(%9)%5<%3> %5(%9%3)

// Detect possible function modifiers
#define e@[%5]
#define statice@[%5] %5_static()
#define stocke@[%5] %5_stock()
#define publice@[%5] %5_public()
#define conste@[%5] %5_const()
#define forwarde@[%5] %5_forward()
#define s@:h@:i@(%0\32;[%5](%8)) %8(%0)
#define h@:i@(%0\32;%2[%5](%8)) %0e@[%5]s@:h@:i@(%2[%5](%8))
#define i@(%0[%5](%8)) %8(%0)
#define y@:%8(%1)%2[%5] s@:h@:i@(%1[%5](%8))%2[%5]

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
// "PARSE@WHATEVER" macros will stop triggering and the "PARSE@()" macro will
// trigger instead to combine all the tag macros.
#define PARSE@(%9,%0,%1,%2,%3,%4)(%5)%6(%7) __:z@:m@:n@[%9]%0%1%2%3%4(%5)%6(|||%7)$

// Return types.
#define PARSE@RET_VOD:%8(%9-%7-,%0,%1,%2,%3,%4)         PARSE@%8(g@:p@-%7-,%0,%1,%2,%3,%4)       // void:
#define PARSE@RET_VOD_STR:%8(%9-%7-,%0,%1,%2,%3,%4)     PARSE@%8(g@:k@:p@-%7-,%0,%1,%2,%3,%4)    // void: string:
#define PARSE@RET_STR_VOD                       PARSE@RET_VOD_STR
#define PARSE@RET_VOD_TAG:%8(%9-%7-,%0,%1,%2,%3,%4)     PARSE@%8(g@:l@:p@-%7-,%0,%1,%2,%3,%4)    // void: ???:
#define PARSE@RET_TAG_VOD                       PARSE@RET_VOD_TAG
#define PARSE@RET_VOD_STR_TAG:%8(%9-%7-,%0,%1,%2,%3,%4) PARSE@%8(g@:k@:l@:p@-%7-,%0,%1,%2,%3,%4) // void: string: ???:
#define PARSE@RET_STR_VOD_TAG                   PARSE@RET_VOD_STR_TAG
#define PARSE@RET_VOD_TAG_STR                   PARSE@RET_VOD_STR_TAG
#define PARSE@RET_STR_TAG_VOD                   PARSE@RET_VOD_STR_TAG
#define PARSE@RET_TAG_VOD_STR                   PARSE@RET_VOD_STR_TAG
#define PARSE@RET_TAG_STR_VOD                   PARSE@RET_VOD_STR_TAG
#define PARSE@RET_TAG:%8(%9-%7-,%0,%1,%2,%3,%4)         PARSE@%8(l@:p@-%7-,%0,%1,%2,%3,%4)       // ???:
#define PARSE@RET_STR:%8(%9-%7-,%0,%1,%2,%3,%4)         PARSE@%8(k@:p@-%7-,%0,%1,%2,%3,%4)       // string:
#define PARSE@RET_STR_TAG:%8(%9-%7-,%0,%1,%2,%3,%4)     PARSE@%8(k@:l@:p@-%7-,%0,%1,%2,%3,%4)    // string: ???:
#define PARSE@RET_TAG_STR                       PARSE@RET_STR_TAG

#define PARSE@QAL:%8(%9-%7-,%0,%1,%2,%3,%4) PARSE@%8(%9-y@:-,%0,%1,%2,%3,%4)       // stock, const, etc.
#define PARSE@QUALIFIERS PARSE@QAL

#define PARSE@RET_VOID            PARSE@RET_VOD
#define PARSE@RET_VOID_STRING     PARSE@RET_VOD_STR
#define PARSE@RET_STRING_VOID     PARSE@RET_VOD_STR
#define PARSE@RET_VOID_TAG        PARSE@RET_VOD_TAG
#define PARSE@RET_TAG_VOID        PARSE@RET_VOD_TAG
#define PARSE@RET_VOID_STRING_TAG PARSE@RET_VOD_STR_TAG
#define PARSE@RET_STRING_VOID_TAG PARSE@RET_VOD_STR_TAG
#define PARSE@RET_VOID_TAG_STRING PARSE@RET_VOD_STR_TAG
#define PARSE@RET_STRING_TAG_VOID PARSE@RET_VOD_STR_TAG
#define PARSE@RET_TAG_VOID_STRING PARSE@RET_VOD_STR_TAG
#define PARSE@RET_TAG_STRING_VOID PARSE@RET_VOD_STR_TAG
#define PARSE@RET_STRING          PARSE@RET_STR
#define PARSE@RET_STRING_TAG      PARSE@RET_STR_TAG
#define PARSE@RET_TAG_STRING      PARSE@RET_STR_TAG

// Enforce array lengths.
#define PARSE@LEN:%8(%9,%0,a@:x@[%1],%2,%3,%4) PARSE@%8(%9,%0,a@:x@[%1LENGTH_REQUIRED{%2%4}],%2,%3,%4)
#define PARSE@LENGTH PARSE@LEN

// All the parameter types that can be detected (hopefully all).  "tag" macros
// will also match multi-tag types such as "{tag, _}:..."
#define PARSE@ARR:%8(%9,%0,%1,%2,%3,%4)             PARSE@%8(%9,%0,a@:x@[],%2,%3,%4)          // a[]
#define PARSE@NUM_TAG:%8(%9,%0,%1,%2,%3,%4)         PARSE@%8(%9,%0,%1,%2,%3,o@:x@[t@:])       // tag:a
#define PARSE@TAG_NUM                           PARSE@NUM_TAG
#define PARSE@STR:%8(%9,%0,%1,%2,%3,%4)             PARSE@%8(%9,w@:x@[],%1,%2,%3,%4)          // string:a[]
#define PARSE@STR_DEF:%8(%9,%0,%1,%2,%3,%4)         PARSE@%8(%9,w@:x@[b@:],%1,%2,%3,%4)       // string:a[] = "hi"
#define PARSE@DEF_STR                           PARSE@STR_DEF
#define PARSE@NUM:%8(%9,%0,%1,%2,%3,%4)             PARSE@%8(%9,%0,%1,%2,%3,o@:x@[])          // a
#define PARSE@ARR_TAG:%8(%9,%0,%1,%2,%3,%4)         PARSE@%8(%9,%0,a@:x@[t@:],%2,%3,%4)       // tag:a[]
#define PARSE@TAG_ARR                           PARSE@ARR_TAG
#define PARSE@NUM_TAG_DEF:%8(%9,%0,%1,%2,%3,%4)     PARSE@%8(%9,%0,%1,%2,%3,o@:x@[t@:d@:])    // tag:a = 5
#define PARSE@NUM_DEF_TAG                       PARSE@NUM_TAG_DEF
#define PARSE@TAG_NUM_DEF                       PARSE@NUM_TAG_DEF
#define PARSE@DEF_NUM_TAG                       PARSE@NUM_TAG_DEF
#define PARSE@TAG_DEF_NUM                       PARSE@NUM_TAG_DEF
#define PARSE@DEF_TAG_NUM                       PARSE@NUM_TAG_DEF
#define PARSE@NUM_DEF:%8(%9,%0,%1,%2,%3,%4)         PARSE@%8(%9,%0,%1,%2,%3,o@:x@[d@:])       // a = 5
#define PARSE@DEF_NUM                           PARSE@NUM_DEF
#define PARSE@REF:%8(%9,%0,%1,%2,%3,%4)             PARSE@%8(%9,%0,%1,r@:x@[],%3,%4)          // &a
#define PARSE@REF_TAG:%8(%9,%0,%1,%2,%3,%4)         PARSE@%8(%9,%0,%1,r@:x@[t@:],%3,%4)       // &tag:a
#define PARSE@TAG_REF                           PARSE@REF_TAG
#define PARSE@REF_DEF:%8(%9,%0,%1,%2,%3,%4)         PARSE@%8(%9,%0,%1,r@:x@[d@:],%3,%4)       // &a = 5
#define PARSE@DEF_REF                           PARSE@REF_DEF
#define PARSE@REF_TAG_DEF:%8(%9,%0,%1,%2,%3,%4)     PARSE@%8(%9,%0,%1,r@:x@[t@:d@:],%3,%4)    // &tag:a = 5
#define PARSE@REF_DEF_TAG                       PARSE@REF_TAG_DEF
#define PARSE@TAG_REF_DEF                       PARSE@REF_TAG_DEF
#define PARSE@TAG_DEF_REF                       PARSE@REF_TAG_DEF
#define PARSE@DEF_REF_TAG                       PARSE@REF_TAG_DEF
#define PARSE@DEF_TAG_REF                       PARSE@REF_TAG_DEF
#define PARSE@EXT:%8(%9,%0,%1,%2,%3,%4)             PARSE@%8(%9,%0,%1,%2,v@:x@[],%4)          // ...
#define PARSE@EXT_TAG:%8(%9,%0,%1,%2,%3,%4)         PARSE@%8(%9,%0,%1,%2,v@:x@[t@:],%4)       // tag:...
#define PARSE@TAG_EXT                           PARSE@EXT_TAG
#define PARSE@ARR_CST:%8(%9,%0,%1,%2,%3,%4)         PARSE@%8(%9,%0,a@:x@[c@:],%2,%3,%4)       // const a[]
#define PARSE@CST_ARR                           PARSE@ARR_CST
#define PARSE@NUM_CST_TAG:%8(%9,%0,%1,%2,%3,%4)     PARSE@%8(%9,%0,%1,%2,%3,o@:x@[c@:t@:])    // const tag:a
#define PARSE@NUM_TAG_CST                       PARSE@NUM_CST_TAG
#define PARSE@TAG_NUM_CST                       PARSE@NUM_CST_TAG
#define PARSE@TAG_CST_NUM                       PARSE@NUM_CST_TAG
#define PARSE@CST_NUM_TAG                       PARSE@NUM_CST_TAG
#define PARSE@CST_TAG_NUM                       PARSE@NUM_CST_TAG
#define PARSE@STR_CST:%8(%9,%0,%1,%2,%3,%4)         PARSE@%8(%9,w@:x@[c@:],%1,%2,%3,%4)       // const string:a[]
#define PARSE@CST_STR                           PARSE@STR_CST
#define PARSE@STR_CST_DEF:%8(%9,%0,%1,%2,%3,%4)     PARSE@%8(%9,w@:x@[c@:b@:],%1,%2,%3,%4)    // const string:a[] = "hi"
#define PARSE@CST_DEF_STR                       PARSE@STR_CST_DEF
#define PARSE@CST_STR_DEF                       PARSE@STR_CST_DEF
#define PARSE@DEF_CST_STR                       PARSE@STR_CST_DEF
#define PARSE@DEF_STR_CST                       PARSE@STR_CST_DEF
#define PARSE@STR_DEF_CST                       PARSE@STR_CST_DEF
#define PARSE@NUM_CST:%8(%9,%0,%1,%2,%3,%4)         PARSE@%8(%9,%0,%1,%2,%3,o@:x@[c@:])       // const a
#define PARSE@CST_NUM                           PARSE@NUM_CST
#define PARSE@ARR_CST_TAG:%8(%9,%0,%1,%2,%3,%4)     PARSE@%8(%9,%0,a@:x@[c@:t@:],%2,%3,%4)    // const tag:a[]
#define PARSE@CST_TAG_ARR                       PARSE@ARR_CST_TAG
#define PARSE@CST_ARR_TAG                       PARSE@ARR_CST_TAG
#define PARSE@ARR_TAG_CST                       PARSE@ARR_CST_TAG
#define PARSE@TAG_CST_ARR                       PARSE@ARR_CST_TAG
#define PARSE@TAG_ARR_CST                       PARSE@ARR_CST_TAG
#define PARSE@NUM_CST_TAG_DEF:%8(%9,%0,%1,%2,%3,%4) PARSE@%8(%9,%0,%1,%2,%3,o@:x@[c@:t@:d@:]) // const tag:a = 5
#define PARSE@NUM_CST_DEF_TAG                   PARSE@NUM_CST_TAG_DEF
#define PARSE@NUM_DEF_CST_TAG                   PARSE@NUM_CST_TAG_DEF
#define PARSE@NUM_DEF_TAG_CST                   PARSE@NUM_CST_TAG_DEF
#define PARSE@NUM_TAG_CST_DEF                   PARSE@NUM_CST_TAG_DEF
#define PARSE@NUM_TAG_DEF_CST                   PARSE@NUM_CST_TAG_DEF
#define PARSE@CST_NUM_TAG_DEF                   PARSE@NUM_CST_TAG_DEF
#define PARSE@CST_NUM_DEF_TAG                   PARSE@NUM_CST_TAG_DEF
#define PARSE@DEF_NUM_CST_TAG                   PARSE@NUM_CST_TAG_DEF
#define PARSE@DEF_NUM_TAG_CST                   PARSE@NUM_CST_TAG_DEF
#define PARSE@TAG_NUM_CST_DEF                   PARSE@NUM_CST_TAG_DEF
#define PARSE@TAG_NUM_DEF_CST                   PARSE@NUM_CST_TAG_DEF
#define PARSE@CST_TAG_NUM_DEF                   PARSE@NUM_CST_TAG_DEF
#define PARSE@CST_DEF_NUM_TAG                   PARSE@NUM_CST_TAG_DEF
#define PARSE@DEF_CST_NUM_TAG                   PARSE@NUM_CST_TAG_DEF
#define PARSE@DEF_TAG_NUM_CST                   PARSE@NUM_CST_TAG_DEF
#define PARSE@TAG_CST_NUM_DEF                   PARSE@NUM_CST_TAG_DEF
#define PARSE@TAG_DEF_NUM_CST                   PARSE@NUM_CST_TAG_DEF
#define PARSE@CST_TAG_DEF_NUM                   PARSE@NUM_CST_TAG_DEF
#define PARSE@CST_DEF_TAG_NUM                   PARSE@NUM_CST_TAG_DEF
#define PARSE@DEF_CST_TAG_NUM                   PARSE@NUM_CST_TAG_DEF
#define PARSE@DEF_TAG_CST_NUM                   PARSE@NUM_CST_TAG_DEF
#define PARSE@TAG_CST_DEF_NUM                   PARSE@NUM_CST_TAG_DEF
#define PARSE@TAG_DEF_CST_NUM                   PARSE@NUM_CST_TAG_DEF
#define PARSE@NUM_CST_DEF:%8(%9,%0,%1,%2,%3,%4)     PARSE@%8(%9,%0,%1,%2,%3,o@:x@[c@:d@:])    // const a = 5
#define PARSE@CST_NUM_DEF                       PARSE@NUM_CST_DEF
#define PARSE@CST_DEF_NUM                       PARSE@NUM_CST_DEF
#define PARSE@DEF_CST_NUM                       PARSE@NUM_CST_DEF
#define PARSE@DEF_NUM_CST                       PARSE@NUM_CST_DEF
#define PARSE@NUM_DEF_CST                       PARSE@NUM_CST_DEF

// Less short names of macros.
#define PARSE@ARRAY                    PARSE@ARR
#define PARSE@NUMBER_TAG               PARSE@NUM_TAG
#define PARSE@TAG_NUMBER               PARSE@NUM_TAG
#define PARSE@STRING                   PARSE@STR
#define PARSE@DEFAULT_STRING           PARSE@STR_DEF
#define PARSE@STRING_DEFAULT           PARSE@STR_DEF
#define PARSE@NUMBER                   PARSE@NUM
#define PARSE@TAG_ARRAY                PARSE@ARR_TAG
#define PARSE@ARRAY_TAG                PARSE@ARR_TAG
#define PARSE@NUMBER_TAG_DEFAULT       PARSE@NUM_TAG_DEF
#define PARSE@NUMBER_DEFAULT_TAG       PARSE@NUM_TAG_DEF
#define PARSE@TAG_NUMBER_DEFAULT       PARSE@NUM_TAG_DEF
#define PARSE@DEFAULT_NUMBER_TAG       PARSE@NUM_TAG_DEF
#define PARSE@TAG_DEFAULT_NUMBER       PARSE@NUM_TAG_DEF
#define PARSE@DEFAULT_TAG_NUMBER       PARSE@NUM_TAG_DEF
#define PARSE@NUMBER_DEFAULT           PARSE@NUM_DEF
#define PARSE@DEFAULT_NUMBER           PARSE@NUM_DEF
#define PARSE@REFERENCE                PARSE@REF
#define PARSE@TAG_REFERENCE            PARSE@REF_TAG
#define PARSE@REFERENCE_TAG            PARSE@REF_TAG
#define PARSE@DEFAULT_REFERENCE        PARSE@REF_DEF
#define PARSE@REFERENCE_DEFAULT        PARSE@REF_DEF
#define PARSE@TAG_REFERENCE_DEFAULT    PARSE@REF_TAG_DEF
#define PARSE@TAG_DEFAULT_REFERENCE    PARSE@REF_TAG_DEF
#define PARSE@REFERENCE_TAG_DEFAULT    PARSE@REF_TAG_DEF
#define PARSE@REFERENCE_DEFAULT_TAG    PARSE@REF_TAG_DEF
#define PARSE@DEFAULT_REFERENCE_TAG    PARSE@REF_TAG_DEF
#define PARSE@DEFAULT_TAG_REFERENCE    PARSE@REF_TAG_DEF
#define PARSE@VARARG                   PARSE@EXT
#define PARSE@VARARG_TAG               PARSE@EXT_TAG
#define PARSE@TAG_VARARG               PARSE@EXT_TAG
#define PARSE@CONST_ARRAY              PARSE@ARR_CST
#define PARSE@ARRAY_CONST              PARSE@ARR_CST
#define PARSE@NUMBER_CONST_TAG         PARSE@NUM_CST_TAG
#define PARSE@NUMBER_TAG_CONST         PARSE@NUM_CST_TAG
#define PARSE@CONST_NUMBER_TAG         PARSE@NUM_CST_TAG
#define PARSE@TAG_NUMBER_CONST         PARSE@NUM_CST_TAG
#define PARSE@CONST_TAG_NUMBER         PARSE@NUM_CST_TAG
#define PARSE@TAG_CONST_NUMBER         PARSE@NUM_CST_TAG
#define PARSE@CONST_STRING             PARSE@STR_CST
#define PARSE@STRING_CONST             PARSE@STR_CST
#define PARSE@CONST_DEFAULT_STRING     PARSE@STR_CST_DEF
#define PARSE@CONST_STRING_DEFAULT     PARSE@STR_CST_DEF
#define PARSE@DEFAULT_CONST_STRING     PARSE@STR_CST_DEF
#define PARSE@DEFAULT_STRING_CONST     PARSE@STR_CST_DEF
#define PARSE@STRING_CONST_DEFAULT     PARSE@STR_CST_DEF
#define PARSE@STRING_DEFAULT_CONST     PARSE@STR_CST_DEF
#define PARSE@CONST_NUMBER             PARSE@NUM_CST
#define PARSE@NUMBER_CONST             PARSE@NUM_CST
#define PARSE@CONST_TAG_ARRAY          PARSE@ARR_CST_TAG
#define PARSE@CONST_ARRAY_TAG          PARSE@ARR_CST_TAG
#define PARSE@ARRAY_CONST_TAG          PARSE@ARR_CST_TAG
#define PARSE@ARRAY_TAG_CONST          PARSE@ARR_CST_TAG
#define PARSE@TAG_CONST_ARRAY          PARSE@ARR_CST_TAG
#define PARSE@TAG_ARRAY_CONST          PARSE@ARR_CST_TAG
#define PARSE@NUMBER_CONST_TAG_DEFAULT PARSE@NUM_CST_TAG_DEF
#define PARSE@NUMBER_CONST_DEFAULT_TAG PARSE@NUM_CST_TAG_DEF
#define PARSE@NUMBER_DEFAULT_CONST_TAG PARSE@NUM_CST_TAG_DEF
#define PARSE@NUMBER_DEFAULT_TAG_CONST PARSE@NUM_CST_TAG_DEF
#define PARSE@NUMBER_TAG_CONST_DEFAULT PARSE@NUM_CST_TAG_DEF
#define PARSE@NUMBER_TAG_DEFAULT_CONST PARSE@NUM_CST_TAG_DEF
#define PARSE@CONST_NUMBER_TAG_DEFAULT PARSE@NUM_CST_TAG_DEF
#define PARSE@CONST_NUMBER_DEFAULT_TAG PARSE@NUM_CST_TAG_DEF
#define PARSE@DEFAULT_NUMBER_CONST_TAG PARSE@NUM_CST_TAG_DEF
#define PARSE@DEFAULT_NUMBER_TAG_CONST PARSE@NUM_CST_TAG_DEF
#define PARSE@TAG_NUMBER_CONST_DEFAULT PARSE@NUM_CST_TAG_DEF
#define PARSE@TAG_NUMBER_DEFAULT_CONST PARSE@NUM_CST_TAG_DEF
#define PARSE@CONST_TAG_NUMBER_DEFAULT PARSE@NUM_CST_TAG_DEF
#define PARSE@CONST_DEFAULT_NUMBER_TAG PARSE@NUM_CST_TAG_DEF
#define PARSE@DEFAULT_CONST_NUMBER_TAG PARSE@NUM_CST_TAG_DEF
#define PARSE@DEFAULT_TAG_NUMBER_CONST PARSE@NUM_CST_TAG_DEF
#define PARSE@TAG_CONST_NUMBER_DEFAULT PARSE@NUM_CST_TAG_DEF
#define PARSE@TAG_DEFAULT_NUMBER_CONST PARSE@NUM_CST_TAG_DEF
#define PARSE@CONST_TAG_DEFAULT_NUMBER PARSE@NUM_CST_TAG_DEF
#define PARSE@CONST_DEFAULT_TAG_NUMBER PARSE@NUM_CST_TAG_DEF
#define PARSE@DEFAULT_CONST_TAG_NUMBER PARSE@NUM_CST_TAG_DEF
#define PARSE@DEFAULT_TAG_CONST_NUMBER PARSE@NUM_CST_TAG_DEF
#define PARSE@TAG_CONST_DEFAULT_NUMBER PARSE@NUM_CST_TAG_DEF
#define PARSE@TAG_DEFAULT_CONST_NUMBER PARSE@NUM_CST_TAG_DEF
#define PARSE@CONST_NUMBER_DEFAULT     PARSE@NUM_CST_DEF
#define PARSE@CONST_DEFAULT_NUMBER     PARSE@NUM_CST_DEF
#define PARSE@DEFAULT_CONST_NUMBER     PARSE@NUM_CST_DEF
#define PARSE@DEFAULT_NUMBER_CONST     PARSE@NUM_CST_DEF
#define PARSE@NUMBER_CONST_DEFAULT     PARSE@NUM_CST_DEF
#define PARSE@NUMBER_DEFAULT_CONST     PARSE@NUM_CST_DEF

// Multi-dimensional arrays.
#define PARSE@ARR_MUL:%8(%9,%0,%1,%2,%3,%4)         PARSE@%8(%9,%0,a@:x@[f@:],%2,%3,%4)       // a[][]
#define PARSE@MUL_ARR                           PARSE@ARR_MUL
#define PARSE@ARR_TAG_MUL:%8(%9,%0,%1,%2,%3,%4)     PARSE@%8(%9,%0,a@:x@[f@:t@:],%2,%3,%4)    // tag:a[][]
#define PARSE@TAG_MUL_ARR                       PARSE@ARR_TAG_MUL
#define PARSE@TAG_ARR_MUL                       PARSE@ARR_TAG_MUL
#define PARSE@ARR_MUL_TAG                       PARSE@ARR_TAG_MUL
#define PARSE@MUL_ARR_TAG                       PARSE@ARR_TAG_MUL
#define PARSE@MUL_TAG_ARR                       PARSE@ARR_TAG_MUL
#define PARSE@ARR_CST_MUL:%8(%9,%0,%1,%2,%3,%4)     PARSE@%8(%9,%0,a@:x@[f@:c@:],%2,%3,%4)    // const a[][]
#define PARSE@CST_MUL_ARR                       PARSE@ARR_CST_MUL
#define PARSE@CST_ARR_MUL                       PARSE@ARR_CST_MUL
#define PARSE@ARR_MUL_CST                       PARSE@ARR_CST_MUL
#define PARSE@MUL_CST_ARR                       PARSE@ARR_CST_MUL
#define PARSE@MUL_ARR_CST                       PARSE@ARR_CST_MUL
#define PARSE@ARR_CST_TAG_MUL:%8(%9,%0,%1,%2,%3,%4) PARSE@%8(%9,%0,a@:x@[f@:c@:t@:],%2,%3,%4) // const tag:a[][]
#define PARSE@CST_TAG_ARR_MUL                   PARSE@ARR_CST_TAG_MUL
#define PARSE@CST_TAG_MUL_ARR                   PARSE@ARR_CST_TAG_MUL
#define PARSE@CST_ARR_TAG_MUL                   PARSE@ARR_CST_TAG_MUL
#define PARSE@CST_ARR_MUL_TAG                   PARSE@ARR_CST_TAG_MUL
#define PARSE@CST_MUL_ARR_TAG                   PARSE@ARR_CST_TAG_MUL
#define PARSE@CST_MUL_TAG_ARR                   PARSE@ARR_CST_TAG_MUL
#define PARSE@TAG_CST_ARR_MUL                   PARSE@ARR_CST_TAG_MUL
#define PARSE@TAG_CST_MUL_ARR                   PARSE@ARR_CST_TAG_MUL
#define PARSE@ARR_CST_MUL_TAG                   PARSE@ARR_CST_TAG_MUL
#define PARSE@MUL_CST_ARR_TAG                   PARSE@ARR_CST_TAG_MUL
#define PARSE@MUL_CST_TAG_ARR                   PARSE@ARR_CST_TAG_MUL
#define PARSE@TAG_ARR_CST_MUL                   PARSE@ARR_CST_TAG_MUL
#define PARSE@TAG_MUL_CST_ARR                   PARSE@ARR_CST_TAG_MUL
#define PARSE@ARR_TAG_CST_MUL                   PARSE@ARR_CST_TAG_MUL
#define PARSE@ARR_MUL_CST_TAG                   PARSE@ARR_CST_TAG_MUL
#define PARSE@MUL_ARR_CST_TAG                   PARSE@ARR_CST_TAG_MUL
#define PARSE@MUL_TAG_CST_ARR                   PARSE@ARR_CST_TAG_MUL
#define PARSE@TAG_ARR_MUL_CST                   PARSE@ARR_CST_TAG_MUL
#define PARSE@TAG_MUL_ARR_CST                   PARSE@ARR_CST_TAG_MUL
#define PARSE@ARR_TAG_MUL_CST                   PARSE@ARR_CST_TAG_MUL
#define PARSE@ARR_MUL_TAG_CST                   PARSE@ARR_CST_TAG_MUL
#define PARSE@MUL_ARR_TAG_CST                   PARSE@ARR_CST_TAG_MUL
#define PARSE@MUL_TAG_ARR_CST                   PARSE@ARR_CST_TAG_MUL
#define PARSE@ARRAY_MULTI           PARSE@ARR_MUL
#define PARSE@MULTI_ARRAY           PARSE@ARR_MUL
#define PARSE@TAG_ARRAY_MULTI       PARSE@ARR_TAG_MUL
#define PARSE@TAG_MULTI_ARRAY       PARSE@ARR_TAG_MUL
#define PARSE@ARRAY_TAG_MULTI       PARSE@ARR_TAG_MUL
#define PARSE@ARRAY_MULTI_TAG       PARSE@ARR_TAG_MUL
#define PARSE@MULTI_ARRAY_TAG       PARSE@ARR_TAG_MUL
#define PARSE@MULTI_TAG_ARRAY       PARSE@ARR_TAG_MUL
#define PARSE@CONST_MULTI_ARRAY     PARSE@ARR_CST_MUL
#define PARSE@CONST_ARRAY_MULTI     PARSE@ARR_CST_MUL
#define PARSE@ARRAY_CONST_MULTI     PARSE@ARR_CST_MUL
#define PARSE@ARRAY_MULTI_CONST     PARSE@ARR_CST_MUL
#define PARSE@MULTI_CONST_ARRAY     PARSE@ARR_CST_MUL
#define PARSE@MULTI_ARRAY_CONST     PARSE@ARR_CST_MUL
#define PARSE@CONST_TAG_ARRAY_MULTI PARSE@ARR_CST_TAG_MUL
#define PARSE@CONST_TAG_MULTI_ARRAY PARSE@ARR_CST_TAG_MUL
#define PARSE@CONST_ARRAY_TAG_MULTI PARSE@ARR_CST_TAG_MUL
#define PARSE@CONST_ARRAY_MULTI_TAG PARSE@ARR_CST_TAG_MUL
#define PARSE@CONST_MULTI_ARRAY_TAG PARSE@ARR_CST_TAG_MUL
#define PARSE@CONST_MULTI_TAG_ARRAY PARSE@ARR_CST_TAG_MUL
#define PARSE@TAG_CONST_ARRAY_MULTI PARSE@ARR_CST_TAG_MUL
#define PARSE@TAG_CONST_MULTI_ARRAY PARSE@ARR_CST_TAG_MUL
#define PARSE@ARRAY_CONST_TAG_MULTI PARSE@ARR_CST_TAG_MUL
#define PARSE@ARRAY_CONST_MULTI_TAG PARSE@ARR_CST_TAG_MUL
#define PARSE@MULTI_CONST_ARRAY_TAG PARSE@ARR_CST_TAG_MUL
#define PARSE@MULTI_CONST_TAG_ARRAY PARSE@ARR_CST_TAG_MUL
#define PARSE@TAG_ARRAY_CONST_MULTI PARSE@ARR_CST_TAG_MUL
#define PARSE@TAG_MULTI_CONST_ARRAY PARSE@ARR_CST_TAG_MUL
#define PARSE@ARRAY_TAG_CONST_MULTI PARSE@ARR_CST_TAG_MUL
#define PARSE@ARRAY_MULTI_CONST_TAG PARSE@ARR_CST_TAG_MUL
#define PARSE@MULTI_ARRAY_CONST_TAG PARSE@ARR_CST_TAG_MUL
#define PARSE@MULTI_TAG_CONST_ARRAY PARSE@ARR_CST_TAG_MUL
#define PARSE@TAG_ARRAY_MULTI_CONST PARSE@ARR_CST_TAG_MUL
#define PARSE@TAG_MULTI_ARRAY_CONST PARSE@ARR_CST_TAG_MUL
#define PARSE@ARRAY_TAG_MULTI_CONST PARSE@ARR_CST_TAG_MUL
#define PARSE@ARRAY_MULTI_TAG_CONST PARSE@ARR_CST_TAG_MUL
#define PARSE@MULTI_ARRAY_TAG_CONST PARSE@ARR_CST_TAG_MUL
#define PARSE@MULTI_TAG_ARRAY_CONST PARSE@ARR_CST_TAG_MUL

