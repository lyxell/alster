local types = table.concat({
    "FILE", "bool", "char", "const", "double", "auto", "float", "int",
    "size_t", "void", "struct", "enum", "char32_t"
}, "|")

local keywords = table.concat({
    "break", "continue", "else", "for", "return", "if",
    "while", "switch", "case", "default", "using"
}, "|")

local punctuators = table.concat({
    "[", "]", "(", ")", "{", "}", ".", "->", "++", "--", "&", "*", "+", "-",
    "~", "!", "/", "%", "<<", ">>", "<", ">", "<=", ">=", "==", "!=", "^",
    "|", "&&", "||", "?", ":", ";", "...", "=", "*=", "/=", "%=", "+=", "-=",
    "<<=", ">>=", "&=", "^=", "|=", ",", "#", "##", "<:", ":>", "<%", "%>",
    "%:", "%:%:"
}, "|")

return {
    {"(L|U|u|u8)?\"[^\"\n\x00]*\"", "C_STRING"},
    {"[1-9][0-9]*",                 "C_LITERAL_DECIMAL"},
    {"[0][0-7]*",                   "C_LITERAL_OCTAL"},
    {"[0-9]+",                      "C_INVALID"},
    {"true|false",                  "C_LITERAL_BOOL"},
    {"[/][/][^\0]*[\0]",            "C_SINGLE_LINE_COMMENT"},
    {types,                         "C_TYPE"},
    {keywords,                      "C_KEYWORD"}
    {punctuators,                   "C_PUNCTUATOR"},
    {"[*]*[a-zA-Z_][a-zA-Z_0-9]*",  "C_IDENTIFIER"},
}
