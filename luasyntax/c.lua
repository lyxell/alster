local types = table.concat({
    "FILE", "bool", "char", "const", "double", "auto", "float", "int",
    "size_t", "void", "struct", "enum", "char32_t"
}, "|")

local keywords = table.concat({
    "break", "continue", "else", "for", "return", "if",
    "while", "switch", "case", "default", "using"
}, "|")

local operators = table.concat({
    "[", "]", "(", ")", "{", "}", ".", "->", "++", "--", "&", "*", "+", "-",
    "~", "!", "/", "%", "<<", ">>", "<", ">", "<=", ">=", "==", "!=", "^",
    "|", "&&", "||", "?", ":", ";", "...", "=", "*=", "/=", "%=", "+=", "-=",
    "<<=", ">>=", "&=", "^=", "|=", ",", "#", "##", "<:", ":>", "<%", "%>",
    "%:", "%:%:"
}, "|")

return {
    ["(L|U|u|u8)?\"[^\"\n\x00]*\""] = function(str)
        return {str, C_STRING}
    end,
    ["[1-9][0-9]*"] = function(str)
        return {str, C_LITERAL_DECIMAL}
    end,
    ["[0][0-7]*"] = function(str)
        return {str, C_LITERAL_OCTAL}
    end,
    ["[0-9]+"] = function(str)
        return {str, C_INVALID}
    end,
    ["true|false"] = function(str)
        return {str, C_LITERAL_BOOL}
    end,
    ["[/][/][^\0]*[\0]"] = function(str)
        return {str, C_SINGLE_LINE_COMMENT}
    end,
    [types] = function(str)
        return {str, C_TYPE}
    end,
    [keywords] = function(str)
        return {str, C_KEYWORD}
    end,
    [operators] = function(str)
        return {str, C_PUNCTUATOR}
    end,
    ["[*]*[a-zA-Z_][a-zA-Z_0-9]*"] = function(str)
        return {str, C_IDENTIFIER}
    end,
    ["*"] = function(str)
        return {str, C_INVALID}
    end
}
