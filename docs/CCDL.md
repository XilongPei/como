
# BNF rules of CCDL(C++ Component Description Language) Syntax

### Programs

<*compilation_unit*> ::= <*include_or_type_declarations*>? <*import_declaration*>? <*module_declaration*>?

### Declarations

<*include_or_type_declarations*> ::= <*include_or_type_declaration*> | <*include_or_type_declarations*> <*include_or_type_declaration*>

<*include_or_type_declaration*> ::= <*include_declaration*> | <*type_declaration*>

<*include_declaration*> ::= include <*string_literal*>

<*import_declaration*> ::= import <*string_literal*>

<*type_declaration*> ::= <*namespace_declaration*> | <*interface_declaration*> | <*class_declaration*> | <*enum_declaration*>

<*namespace_declaration*> ::= namespace <*identifier*> { <*include_or_type_declarations*> }

<*interface_declaration*> ::= \[ <*interface_attributes*>? \] interface <*identifier*> <*extands_interface*>? <*interface_body*> | interface <*interface_type*> ;

<*interface_attributes*> ::= <*interface_attribute*> | <*interface_attributes*> , <*interface_attribute*>

<*interface_attribute*> ::= <*uuid_attribute*> | <*version_attribute*> | <*description_attribute*> | <*contractblock_attribute*> | <*funcsafetysetting*>

<*uuid_attribute*> ::= uuid ( <*uuid_literal*> )

<*uri_attribute*> ::= uri ( <*uri_literal*> )

<*version_attribute*> ::= version ( <*version_literal*> )

<*description_attribute*> ::= description ( <*string_literal*> )

<*funcsafetysetting*> ::= FuncSafetySetting ( <*string_literal*> )

<*contractblock_attribute*> ::= <*contractblock_m_attribute*> | <*contractblock_s_attribute*>

<*contractblock_m_attribute*> ::= /*@ <*string_literal*>  */

<*contractblock_s_attribute*> ::= //@ <*string_literal*>

<*extends_interface*> ::= : <*interface_type*>

<*interface_body*> ::= { <*interface_member_declarations*>? }

<*interface_member_declarations*> ::= <*interface_member_declaration*> | <*interface_member_declarations*> <*interface_member_declaration*>

<*interface_member_declaration*> ::= <*interface_declaration*> | <*constant_declaration*> | <*abstract_method_declaration*>

<*constant_declaration*> ::= const <*type*> <*identifier*> = <*expression*> ;

<*abstract_method_declaration*>::= <*method_declarator*> ;

<*method_declarator*> ::= <*identifier*> ( <*formal_parameters*>? )

<*formal_parameters*> ::= <*formal_parameter*> | <*formal_parameters*> , <*formal_parameter*>

<*formal_parameter*> ::= <*formal_parameter_attribute*> <*type*> <*identifier*>

<*formal_parameter_attribute*> ::= \[ in | out | in , out | out , callee \]

<*class_declaration*> ::= \[ <*class_attributes*>? \] class <*identifier*> <*class_body*>

<*class_attributes*> ::= <*class_attribute*> | <*class_attributes*> , <*class_attribute*>

<*class_attribute*> ::= <*uuid_attribute*> | <*version_attribute*> | <*description_attribute*> | <*contractblock_attribute*> | <*funcsafetysetting*>

<*class_body*> ::= { <*class_body_declarations*>? }

<*class_body_eclarations*> ::= <*class_body_declaration*> | <*class_body_declarations*> <*class_body_declaration*>

<*class_body_declaration*> ::= <*constructor_declaration*> | <*interface_declaration*>

<*constructor_declaration*> ::= constructor ( <*formal_parameters*>? )

<*interface_declaration*> ::= interface <*interface_name*> ;

<*enum_declaration*> ::= enum <*identifier*> <*enum_body*>

<*enum_body*> ::= { <*enum_body_declarations*>? }

<*enum_body_declarations*> ::= <*enum_body_declaration*> | <*enum_body_declarations*> , <*enum_body_declaration*>

<*enum_body_declaration*> ::= <*enumerators*>

<*enumerators*> ::= <*enumerator*> | <*enumerators*> , <*enumerator*>

<*enumerator*> ::= <*identifier*> | <*identifier*> = <*expression*>

<*module_declaration*> ::= \[ <*module_attributes*>? \] module <*identifier*> <*module_body*>

<*module_attributes*> ::= <*module_attribute*> | <*module_attributes*> , <*module_attribute*>

<*module_attribute*> ::= <*uuid_attribute*> | <*version_attribute*> | <*description_attribute*> | <*uri_attribute*>

<*module_body*> ::= { <*module_body_declarations*>? }

<*module_body_declarations*> ::= <*module_body_declaration*> | <*module_body_declarations*> <*module_body_declaration*>

<*module_body_declaration*> ::= <*include_or_type_declarations*>

### Types

<*type*> ::= <*primitive_type*> | <*reference_type*> | <*pointer_type*>

<*primitive_type*> ::= <*numeric_type*> | Boolean | String | CoclassID | ComponentID | InterfaceID | HANDLE | ECode

<*numeric_type*> ::= <*integral_type*> | <*floating-point_type*>

<*integral_type*> ::= Byte | Short | Integer | Long | Char

<*floating-point_type*> ::= Float | Double

<*reference_type*> ::= <*interface_type*> | <*array_type*> | <*enum_type*>

<*interface_type*> ::= <*type_name*>

<*array_type*> ::= Array < <*type*> >

<*enum_type*> ::= <*type_name*>

<*pointer_type*> ::= <*type*> *

### Expressions

<*expression*> ::= <*inclusive_or_expression*>

<*inclusive_or_expression*> ::= <*exclusive_or_expression*> | <*inclusive_or_expression*> **|** <*exclusive_or_expression*>

<*exclusive_or_expression*> ::= <*and_expression*> | <*exclusive_or_expression*> ^ <*and_expression*>

<*and_expression*> ::= <*shift_expression*> | <*and_expression*> & <*shift_expression*>

<*shift_expression*> ::= <*additive_expression*> | <*shift_expression*> << <*additive_expression*> | <*shift_expression*> >> <*additive_expression*> | <*shift_expression*> >>> <*additive_expression*>

<*additive_expression*> ::= <*multiplicative_expression*> | <*additive_expression*> + <*multiplicative_expression*> | <*additive_expression*> - <*multiplicative_expression*>

<*multiplicative_expression*> ::= <*unary_expression*> | <*multiplicative_expression*> * <*unary_expression*> | <*multiplicative_expression*> / <*unary_expression*> | <*multiplicative_expression*> % <*unary_expression*>

<*unary_expression*> ::= <*postfix_expression*> | + <*unary_expression*> | - <*unary_expression*> | ~ <*unary_expression*> | ! <*unary_expression*>

<*postfix_expression*> ::= <*primary*> | <*expression_name*>

<*primary*> ::= <*literal*> | ( <*expression*> )

### Tokens

<*namespace_name*> ::= <*identifier*> | <*namespace_name*> :: <*identifier*>

<*type_name*> ::= <*identifier*> | <*namespace_name*> :: <*identifier*>

<*expression_name*> ::= <*identifier*> | <*ambiguous_name*> . <*identifier*>

<*ambiguous_name*>::= <*identifier*> | <*ambiguous_name*>. <*identifier*>

<*literal*> ::= <*integer_literal*> | <*floating-point_literal*> | <*boolean_literal*> | <*character_literal*> | <*string_literal*>

<*integer_literal*> ::= <*decimal_integer_literal*> | <*hex_integer_literal*> | <*octal_integer_literal*>

<*decimal_integer_literal*> ::= <*decimal_numeral*> <*integer_type_suffix*>?

<*hex_integer_literal*> ::= <*hex_numeral*> <*integer_type_suffix*>?

<*octal_integer_literal*> ::= <*octal_numeral*> <*integer_type_suffix*>?

<*uuid_literal*> ::= <*time_low_literal*> - <*time_mid_literal*> - <*time_high_and_version_literal*> - <*clock_seq_and_reserved_literal*> <*clock_seq_low_literal*> - <*node_literal*>

<*time_low_literal*> ::= 4<*hex_octet*>

<*time_mid_literal*> ::= 2<*hex_octet*>

<*time_high_and_version_literal*> ::= 2<*hex_octet*>

<*clock_seq_and_reserved_literal*> ::= 2<*hex_octet*>

<*clock_seq_low_literal*> ::= <*hex_octet*>

<*node_literal*> ::= 6<*hex_octet*>

<*hex_octet*> ::= <*hex_digit*> <*hex_digit*>

<*version_literal*> ::= <*decimal_numeral*> . <*decimal_numeral*> . <*decimal_numeral*>

<*integer_type_suffix*> ::= ll | LL

<*decimal_numeral*> ::= 0 | <*non_zero_digit*> <*digits*>?

<*digits*> ::= <*digit*> | <*digits*> <*digit*>

<*digit*> ::= 0 | <*non_zero_digit*>

<*non_zero_digit*> ::= 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9

<*hex_numeral*> ::= 0 x <*hex_digit*> | 0 X <*hex_digit*> | <*hex_numeral*> <*hex_digit*>

<*hex_digit*> ::= 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | a | b | c | d | e | f | A | B | C | D | E | F

<*octal_numeral*> ::= 0 <*octal_digit*> | <*octal_numeral*> <*octal_digit*>

<*octal_digit*> ::= 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7

<*floating-point_literal*> ::= <*digits*> . <*digits*>? <*exponent_part*>? <*float_type_suffix*>? | <*digits*> <*exponent_part*>? <*float_type_suffix*>?

<*exponent_part*> ::= <*exponent_indicator*> <*signed_integer*>

<*exponent_indicator*> ::= e | E

<*signed_integer*> ::= <*sign*>? <*digits*>

<*sign*> ::= + | -

<*float_type_suffix*> ::= f | F | d | D

<*boolean_literal*> ::= true | false

<*character_literal*> ::= ' <*single_character*> ' | ' <*escape_sequence*> '

<*single_character*> ::= <*input_character*> except ' and \

<*string_literal*> ::= " <*string_characters*>?"

<*string_characters*> ::= <*string_character*> | <*string_characters*> <*string_character*>

<*string_character*> ::= <*input_character*> except " and \ | <*escape_character*>

<*keyword*> ::= Array | Boolean | Byte | coclass | const | description | enum | HANDLE | include | import | Integer | interface | Long | module | namespace | Short | String | uuid | version

### URI

<uri_literal> ::= <scheme> "://" <authority> <path> [ "?" <query> ] [ "#" <fragment> ]

<scheme> ::= "http" | "https" | "ftp" | "file" | ...    ; 多种可能的方案
<authority> ::= [ <userinfo> "@" ] <host> [ ":" <port> ]

<userinfo> ::= <username> [ ":" <password> ]
<username> ::= <unreserved> | <pct-encoded> | <sub-delims>
<password> ::= <unreserved> | <pct-encoded> | <sub-delims>

<host> ::= <IP-literal> | <IPv4address> | <reg-name>
<IP-literal> ::= "[" ( <IPv6address> | <IPvFuture> ) "]"
<IPv4address> ::= <dec-octet> "." <dec-octet> "." <dec-octet> "." <dec-octet>
<dec-octet> ::= DIGIT | %x31-39 DIGIT | "1" 2DIGIT | "2" %x30-34 DIGIT | "25" %x30-35
<reg-name> ::= *( <unreserved> | <pct-encoded> | <sub-delims> )

<port> ::= *DIGIT

<path> ::= "/" <segment> *( "/" <segment> )
<segment> ::= <pchar> *
<pchar> ::= <unreserved> | <pct-encoded> | <sub-delims> | ":" | "@"

<query> ::= *( <pchar> | "/" | "?" )
<fragment> ::= *( <pchar> | "/" | "?" )

<unreserved> ::= ALPHA | DIGIT | "-" | "." | "_" | "~"
<pct-encoded> ::= "%" HEXDIG HEXDIG
<sub-delims> ::= "!" | "$" | "&" | "'" | "(" | ")" | "*" | "+" | "," | ";" | "="

ALPHA ::= %x41-5A | %x61-7A   ; A-Z | a-z
DIGIT ::= %x30-39             ; 0-9
HEXDIG ::= DIGIT | "A" | "B" | "C" | "D" | "E" | "F"

The character set for MIDL is 7-bit ASCII character set. This is the set denoted by <*input_character*>.

The syntax category <*identifier*> consists of strings that must start with a letter - including underscore (_) - followed by any number of letters and digits. Also, <*identifier*> includes none of the keywords given above.

There are two ways to add comments. One is using syntax "/* <*comment_literal*> */" as a block comment. Another is using syntax "// <*comment-literal*>" as a line comment.

#### Type Signature

| Type        | Signature |
|:-----------:|:---------:|
| Byte        |     B     |
| Short       |     S     |
| Integer     |     I     |
| Long        |     J     |
| Float       |     F     |
| Double      |     D     |
| Char        |     C     |
| Boolean     |     Z     |
| String      |     T     |
| HANDLE      |     H     |
| ECode       |     E     |
| CoclassID   |     K     |
| ComponentID |     M     |
| InterfaceID |     U     |
| Triple      |     R     |
| Array       |     [     |
| Pointer     |     *     |
| Reference   |     &     |
| Enum        | Lxx/xx;   |
| Interface   | Lxx/xx;   |

- Type signatures are mainly used when determining function overloading, such as GetSignature() in source code reflection/CMetaInterface.cpp


#### Local Types
A local type is the type which cannot be transact through rpc. Local types include HANDLE, CoclassID, ComponentID, InterfaceID and Array<*local types*>.

#### Reference
[Java Syntax Specification](https://users-cs.au.dk/amoeller/RegAut/JavaBNF.html)
