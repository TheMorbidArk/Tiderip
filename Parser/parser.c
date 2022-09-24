
// @description:                           //

/* ~ INCLUDE ~ */
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "common.h"
#include "utils.h"
#include "unicodeUtf8.h"

/** ParseUnicodeCodePoint
 * 解析unicode码点
 * @param parser
 * @param buf
 */
static void ParseUnicodeCodePoint(Parser *parser, ByteBuffer *buf) {
    uint32_t idx = 0;
    int value = 0;
    uint8_t digit = 0;

//获取数值,u后面跟着4位十六进制数字
    while (idx++ < 4) {
        GetNextChar(parser);
        if (parser->curChar == '\0') {
            LEX_ERROR(parser, "unterminated unicode!");
        }
        if (parser->curChar >= '0' && parser->curChar <= '9') {
            digit = parser->curChar - '0';
        } else if (parser->curChar >= 'a' && parser->curChar <= 'f') {
            digit = parser->curChar - 'a' + 10;
        } else if (parser->curChar >= 'A' && parser->curChar <= 'F') {
            digit = parser->curChar - 'A' + 10;
        } else {
            LEX_ERROR(parser, "invalid unicode!");
        }
        value = value * 16 | digit;
    }

    uint32_t byteNum = GetByteNumOfEncodeUtf8(value);
    ASSERT(byteNum != 0, "utf8 encode bytes should be between 1 and 4!");

    //为代码通用, 下面会直接写buf->datas,在此先写入byteNum个0,以保证事先有byteNum个空间
    ByteBufferFillWrite(parser->vm, buf, 0, byteNum);

    //把value编码为utf8后写入缓冲区buf
    EncodeUtf8(buf->datas + buf->count - byteNum, value);
}

/* ~ GET Token ~ */

typedef struct KeyWordToken {
    char *keyword;    // 关键字
    uint8_t length;     // 关键字长度
    TokenType token;      // Token 类型
} KeyWordToken;

KeyWordToken KeyWordsToken[] = {
        {"var",      3, TOKEN_VAR},
        {"fun",      3, TOKEN_FUN},
        {"if",       2, TOKEN_IF},
        {"else",     4, TOKEN_ELSE},
        {"true",     4, TOKEN_TRUE},
        {"false",    5, TOKEN_FALSE},
        {"while",    5, TOKEN_WHILE},
        {"for",      3, TOKEN_FOR},
        {"break",    5, TOKEN_BREAK},
        {"continue", 8, TOKEN_CONTINUE},
        {"return",   6, TOKEN_RETURN},
        {"null",     4, TOKEN_NULL},
        {"class",    5, TOKEN_CLASS},
        {"is",       2, TOKEN_IS},
        {"static",   6, TOKEN_STATIC},
        {"this",     4, TOKEN_THIS},
        {"super",    5, TOKEN_SUPER},
        {"import",   6, TOKEN_IMPORT},
        {NULL,       0, TOKEN_UNKNOWN}
};

/** IdOrKeyWord
 * 判断 Stat 是否为关键字并返回其对应的 Token
 * @param start 传入待判断的字符
 * @param length 字符串长度
 * @return Stat 对应的 Token
 */
static TokenType IdOrKeyWord(const char *start, uint32_t length) {
    uint32_t idx = 0;
    while (KeyWordsToken[idx].keyword != NULL) {
        if (KeyWordsToken[idx].length == length && memcmp(KeyWordsToken[idx].keyword, start, length) == 0) {
            return KeyWordsToken[idx].token;
        }
        idx++;
    }
    return TOKEN_ID;
}

/** LookAheadChar
 * 向前查看一个字符 "abc" 'a'->'b'
 * @param parser 词法分析器
 * @return char -> nextCharPtr 下一个字符
 */
char LookAheadChar(Parser *parser) {
    return *parser->nextCharPtr;
}

/** GetNextChar
 * 获取下一个字符
 * @param parser  词法分析器
 */
void GetNextChar(Parser *parser) {
    parser->curChar = *parser->nextCharPtr++;
}

/** MatchNextChar
 * 判断下一个字符是否是 expectedChar
 * 如果是 expectedChar 就读进并返回 true
 * 如果不是 expectedChar 就返回 false
 * @param parser 词法分析器
 * @param expectedChar 期望字符
 * @return bool -> true or false
 */
static bool MatchNextChar(Parser *parser, char expectedChar) {
    if (LookAheadChar(parser) == expectedChar) {
        GetNextChar(parser);
        return true;
    }
    return false;
}

/** SkipBlanks
 * 跳过连续的空白字符
 * @param parser 词法分析器
 */
static void SkipBlanks(Parser *parser) {
    while (isspace(parser->curChar)) {
        if (parser->curChar == '\n') {
            parser->curToken.lineNo++;
        }
        GetNextChar(parser);
    }
}

/** ParseId
 * 解析标识符 Id
 * @param parser 词法分析器
 * @param type Token 类型
 */
static void ParseId(Parser *parser, TokenType type) {
    while (isalnum(parser->curChar) || parser->curChar == '_') {
        GetNextChar(parser);
    }

    //nextCharPtr会指向第1个不合法字符的下一个字符,因此-1
    uint32_t length = (uint32_t) (parser->nextCharPtr - parser->curToken.start - 1); //取被解析字符的长度
    if (type != TOKEN_UNKNOWN) {
        parser->curToken.type = type;
    } else {
        parser->curToken.type = IdOrKeyWord(parser->curToken.start, length);
    }
    parser->curToken.length = length;
}

/** ParseString
 * 解析字符串 String
 * @param parser 词法分析器
 */
static void ParseString(Parser *parser) {
    ByteBuffer str;
    ByteBufferInit(&str);
    while (true) {
        GetNextChar(parser);

        // 解析 '\0'
        if (parser->curChar == '\0') {
            LEX_ERROR(parser, "unterminated string!");
        }

        // 解析 ""
        if (parser->curChar == '"') {
            parser->curToken.type = TOKEN_STRING;
            break;
        }

        // 解析 %() 内嵌表达式
        if (parser->curChar == '%') {

            if (!MatchNextChar(parser, '(')) {
                LEX_ERROR(parser, "'%' should followed by '('!");
            }
            if (parser->interpolationExpectRightParenNum > 0) {
                COMPILE_ERROR(parser, "sorry, I don`t support nest interpolate expression!");
            }
            parser->interpolationExpectRightParenNum = 1;
            parser->curToken.type = TOKEN_INTERPOLATION;
            break;
        }

        if (parser->curChar == '\\') {
            GetNextChar(parser);
            //  解析转义字符
            switch (parser->curChar) {
                case '0':
                    ByteBufferAdd(parser->vm, &str, '\0');
                    break;
                case 'a':
                    ByteBufferAdd(parser->vm, &str, '\a');
                    break;
                case 'b':
                    ByteBufferAdd(parser->vm, &str, '\b');
                    break;
                case 'f':
                    ByteBufferAdd(parser->vm, &str, '\f');
                    break;
                case 'n':
                    ByteBufferAdd(parser->vm, &str, '\n');
                    break;
                case 'r':
                    ByteBufferAdd(parser->vm, &str, '\r');
                    break;
                case 't':
                    ByteBufferAdd(parser->vm, &str, '\t');
                    break;
                case 'u':
                    ParseUnicodeCodePoint(parser, &str);
                    break;
                case '"':
                    ByteBufferAdd(parser->vm, &str, '"');
                    break;
                case '\\':
                    ByteBufferAdd(parser->vm, &str, '\\');
                    break;
                default:
                    LEX_ERROR(parser, "unsupport escape \\%c", parser->curChar);
                    break;
            }
        } else {
            // 普通字符串
            ByteBufferAdd(parser->vm, &str, parser->curChar);
        }
    }
    ByteBufferClear(parser->vm, &str);
}

/** SkipALine
 * 跳过一行
 * @param parser 词法分析器
 */
static void SkipALine(Parser *parser) {
    GetNextChar(parser);
    while (parser->curChar != '\0') {
        if (parser->curChar == '\n') {
            parser->curToken.lineNo++;
            GetNextChar(parser);
            break;
        }
    }
}

/** SkipComment
 * 跳过注释
 * @param parser 词法分析器
 */
static void SkipComment(Parser *parser) {
    char nextChar = LookAheadChar(parser);
    if (parser->curChar == '/') {
        SkipALine(parser);
    } else {
        while (nextChar != '*' && nextChar != '\0') {
            GetNextChar(parser);
            if (parser->curChar == '\n') {
                parser->curToken.lineNo++;
            }
            nextChar = LookAheadChar(parser);
        }
        if (MatchNextChar(parser, '*')) {
            if (!MatchNextChar(parser, '/')) {
                LEX_ERROR(parser, "expect '/' after '*' ");
            }
            GetNextChar(parser);
        } else {
            LEX_ERROR(parser, "expect '*/' before file end ");
        }
    }
    SkipBlanks(parser);
}

/** GetNextToken
 * 获得下一个 token , 识别单词的 Token
 * @param parser 词法分析器
 */
void GetNextToken(Parser *parser) {
    // 初始化 Parser->curToken
    parser->preToken = parser->curToken;
    SkipBlanks(parser); // 跳过待识别单词之前的空格
    parser->curToken.type = TOKEN_EOF;
    parser->curToken.length = 0;
    parser->curToken.start = parser->nextCharPtr - 1;

    while (parser->curChar != '\0') {
        switch (parser->curChar) {
            case ',':
                parser->curToken.type = TOKEN_COMMA;
                break;
            case ':':
                parser->curToken.type = TOKEN_COLON;
                break;
            case '(':
                // 判断 () 嵌套
                if (parser->interpolationExpectRightParenNum > 0) {
                    parser->interpolationExpectRightParenNum++;
                }
                parser->curToken.type = TOKEN_LEFT_PAREN;
                break;
            case ')':
                // 判断 () 嵌套
                if (parser->interpolationExpectRightParenNum > 0) {
                    parser->interpolationExpectRightParenNum--;
                    if (parser->interpolationExpectRightParenNum == 0) {
                        ParseString(parser);
                        break;
                    }
                }
                parser->curToken.type = TOKEN_RIGHT_PAREN;
                break;
            case '[':
                parser->curToken.type = TOKEN_LEFT_BRACKET;
                break;
            case ']':
                parser->curToken.type = TOKEN_RIGHT_BRACKET;
                break;
            case '{':
                parser->curToken.type = TOKEN_LEFT_BRACE;
                break;
            case '}':
                parser->curToken.type = TOKEN_RIGHT_BRACE;
                break;
            case '.':
                if (MatchNextChar(parser, '.')) {
                    // ..
                    parser->curToken.type = TOKEN_DOT_DOT;
                } else {
                    // .
                    parser->curToken.type = TOKEN_DOT;
                }
                break;
            case '=':
                if (MatchNextChar(parser, '=')) {
                    parser->curToken.type = TOKEN_EQUAL;
                } else {
                    parser->curToken.type = TOKEN_ASSIGN;
                }
                break;
            case '+':
                parser->curToken.type = TOKEN_ADD;
                break;
            case '-':
                parser->curToken.type = TOKEN_SUB;
                break;
            case '*':
                parser->curToken.type = TOKEN_MUL;
                break;
            case '/':
                if (MatchNextChar(parser, '/') || MatchNextChar(parser, '*')) {
                    // " // " or " /**/ "
                    SkipComment(parser);
                    parser->curToken.start = parser->nextCharPtr - 1;
                    continue;
                } else {
                    parser->curToken.type = TOKEN_DIV;
                }
                break;
            case '%':
                parser->curToken.type = TOKEN_MOD;
                break;
            case '&':
                if (MatchNextChar(parser, '&')) {
                    // &&
                    parser->curToken.type = TOKEN_LOGIC_AND;
                } else {
                    // &s
                    parser->curToken.type = TOKEN_BIT_AND;
                }
                break;
            case '|':
                if (MatchNextChar(parser, '|')) {
                    // ||
                    parser->curToken.type = TOKEN_LOGIC_OR;
                } else {
                    // |
                    parser->curToken.type = TOKEN_BIT_OR;
                }
                break;
            case '~':
                parser->curToken.type = TOKEN_BIT_NOT;
                break;
            case '?':
                parser->curToken.type = TOKEN_QUESTION;
                break;
            case '>':
                if (MatchNextChar(parser, '=')) {
                    // >=
                    parser->curToken.type = TOKEN_GREATE_EQUAL;
                } else if (MatchNextChar(parser, '>')) {
                    // >>
                    parser->curToken.type = TOKEN_BIT_SHIFT_RIGHT;
                } else {
                    // >
                    parser->curToken.type = TOKEN_GREATE;
                }
                break;
            case '<':
                if (MatchNextChar(parser, '=')) {
                    // <=
                    parser->curToken.type = TOKEN_LESS_EQUAL;
                } else if (MatchNextChar(parser, '<')) {
                    // <<
                    parser->curToken.type = TOKEN_BIT_SHIFT_LEFT;
                } else {
                    // <
                    parser->curToken.type = TOKEN_LESS;
                }
                break;
            case '!':
                if (MatchNextChar(parser, '=')) {
                    // !=
                    parser->curToken.type = TOKEN_NOT_EQUAL;
                } else {
                    // !
                    parser->curToken.type = TOKEN_LOGIC_NOT;
                }
                break;
            case '"':
                ParseString(parser);
                break;
            default:
                // 首字符是字母或'_'则是变量名
                if (isalpha(parser->curChar) || parser->curChar == '_') {
                    ParseId(parser, TOKEN_UNKNOWN);  // 解析变量名其余的部分
                } else {
                    // 跳过 '魔数'
                    if (parser->curChar == '#' && MatchNextChar(parser, '!')) {
                        SkipALine(parser);
                        parser->curToken.start = parser->nextCharPtr - 1;  // 重置下一个token起始地址
                        continue;
                    }
                    LEX_ERROR(parser, "unsupport char: \'%c\', quit.", parser->curChar);
                }
                return;
        }
        parser->curToken.length = (uint32_t)(parser->nextCharPtr - parser->curToken.start);
        GetNextChar(parser);
        return;
    }
}

/** MatchToken
 * 若当前token为expected则读入下一个token并返回true,否则不读入token且返回false
 * @param parser 词法分析器
 * @param expected 期望 Token
 * @return bool -> true or false
 */
bool MatchToken(Parser *parser, TokenType expected) {
    if(parser->curToken.type == expected){
        GetNextToken(parser);
        return true;
    }
    return false;
}

/** ConsumeCurToken
 * 断言当前token为expected并读入下一token,否则报错errMsg
 * @param parser 词法分析器
 * @param expected 期望 Token
 * @param errMsg 报错信息
 */
void ConsumeCurToken(Parser* parser, TokenType expected, const char* errMsg){
    if(parser->curToken.type != expected){
        COMPILE_ERROR(parser, errMsg);
    }
    GetNextToken(parser);
}

/** ConsumeNextToken
 * 断言下一个token为expected,否则报错errMsg
 * @param parser 词法分析器
 * @param expected 期望 Token
 * @param errMsg 报错信息
 */
void ConsumeNextToken(Parser* parser, TokenType expected, const char* errMsg) {
    GetNextToken(parser);
    if (parser->curToken.type != expected) {
        COMPILE_ERROR(parser, errMsg);
    }
}

/**
 * 初始化 Parser
 * @param vm
 * @param parser
 * @param file 源文件名
 * @param sourceCode 源码串
 */
void InitParser(VM* vm, Parser* parser, const char* file, const char* sourceCode) {
    parser->file = file;
    parser->sourceCode = sourceCode;
    parser->curChar = *parser->sourceCode;
    parser->nextCharPtr = parser->sourceCode + 1;
    parser->curToken.lineNo = 1;
    parser->curToken.type = TOKEN_UNKNOWN;
    parser->curToken.start = NULL;
    parser->curToken.length = 0;
    parser->preToken = parser->curToken;
    parser->interpolationExpectRightParenNum = 0;
    parser->vm = vm;
}






