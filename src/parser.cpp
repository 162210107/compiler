#include <parser.hpp>
Parser parser;

void Parser::reportError(unsigned int errorType, const wchar_t *expected, const wchar_t *context)
{
    errorHandle.error(errorType, expected, context,
                      lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
}

// 用于错误恢复的函数，若当前符号在s1中，则读取下一符号；若当前符号不在s1中，则报错，接着循环查找下一个在中s1 ∪ s2的符号
int Parser::judge(const unsigned long s1, const unsigned long s2, const unsigned int n, const wchar_t *extra)
{
    if (!(lexer.GetTokenType() & s1)) // 当前符号不在s1中
    {
        errorHandle.error(n, extra, lexer.GetPreWordRow(),
                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        unsigned long s3 = s1 | s2; // 把s2补充进s1

        while (!(lexer.GetTokenType() & s3)) // 循环找到下一个合法的符号
        {
            if (lexer.GetCh() == L'\0')
            {
                // errorHandle.over();
                return 2;
            }
            // return 0;
            lexer.GetWord(); // 继续词法分析
        }
        if (lexer.GetTokenType() & s1)
        {
            return 1;
        }
        else if (lexer.GetTokenType() & s2)
        {
            // 已经获取了下一个匹配的first，所以无需再读
            return -1;
        }
        else
            return 0;
    }
    else
    {
        return 1;
    }
}

// 输出：1是匹配到s1,-1匹配到s2,2是终极
int Parser::judge(const unsigned long s1, const unsigned long s2, const unsigned int n, const wchar_t *extra1, const wchar_t *extra2)
{
    if (!(lexer.GetTokenType() & s1)) // 当前符号不在s1中
    {
        errorHandle.error(n, extra1, extra2, lexer.GetPreWordRow(),
                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        unsigned long s3 = s1 | s2; // 把s2补充进s1

        while (!(lexer.GetTokenType() & s3)) // 循环找到下一个合法的符号
        {
            if (lexer.GetCh() == L'\0')
            {
                // errorHandle.over();
                return 2;
            }
            lexer.GetWord(); // 继续词法分析
        }

        if (lexer.GetTokenType() & s1)
            return 1;
        else if (lexer.GetTokenType() & s2)
        {
            // 已经获取了下一个匹配的first，所以无需再读
            return -1;
        }
        else
            return 0;
    }
    else
        return 1;
}

/*<lop> → =|<>|<|<=|>|>=
<aop> → +|-
<mop> → *|/
<id> → l{l|d}   （注：l表示字母）
<integer> → d{d}*/

/*<statement> → <id> := <exp>
               |if <lexp> then <statement>[else <statement>]
               |while <lexp> do <statement>
               |call <id>（[<exp>{,<exp>}]）
               |<body>
               |read (<id>{，<id>})
               |write (<exp>{,<exp>})*/
void Parser::statement()
{
    // <statement> -> id := <exp>
    if (lexer.GetTokenType() == IDENT)
    {
        lexer.GetWord();
        if (lexer.GetTokenType() == ASSIGN)
        {
            lexer.GetWord();
            exp();
        }
        else if (lexer.GetTokenType() & firstExp)
        {
            errorHandle.error(MISSING, L":=", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            exp();
        }
        else if (lexer.GetTokenType() & EQL)
        {
            errorHandle.error(EXPECT_STH_FIND_ANTH, L":=", L"=", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            lexer.GetWord();
            exp();
        }
        else
        {
            errorHandle.error(ILLEGAL_DEFINE, L"<ident>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
    }
    else if (lexer.GetTokenType() & IF_SYM)
    { // <statement> -> if <lexp> then <statement> [else <statement>]
        lexer.GetWord();
        lexp();
        // system("pause");
        // wcout << lexer.GetTokenType() << endl;
        if (lexer.GetTokenType() & THEN_SYM)
        {
            // system("pause");
            // wcout << (lexer.GetTokenType()&THEN_SYM)<<L"now" << endl;
            lexer.GetWord();
            statement();
            if (lexer.GetTokenType() & ELSE_SYM)
            {
                lexer.GetWord();
                statement();
            }
        }
        else if (lexer.GetTokenType() & firstStatement)
        {
            // system("pause");
            errorHandle.error(MISSING, L"then", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            statement();
            if (lexer.GetTokenType() & ELSE_SYM)
            {
                lexer.GetWord();
                statement();
            }
        }
        else if (lexer.GetTokenType() & ELSE_SYM)
        {
            // system("pause");
            // wcout << lexer.GetTokenType() << endl;
            lexer.GetWord();
            statement();
        }
        else
        {
            errorHandle.error(ILLEGAL_DEFINE, L"<if>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
    }
    else if (lexer.GetTokenType() == WHILE_SYM)
    { // <statement> -> while <lexp> do <statement>
        lexer.GetWord();
        lexp();
        if (lexer.GetTokenType() == DO_SYM)
        {
            lexer.GetWord();
            statement();
        }
        else if (lexer.GetTokenType() & firstStatement)
        {
            errorHandle.error(MISSING, L"do", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            statement();
        }
        else
        {
            errorHandle.error(MISSING, L"do", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
    }
    else if (lexer.GetTokenType() == CALL_SYM)
    { // <statement> -> call id ([{<exp>{,<exp>}])
        lexer.GetWord();
        // <statement> -> call id
        if (lexer.GetTokenType() & IDENT)
        {
            lexer.GetWord();
            if (lexer.GetTokenType() & LPAREN)
            {
                lexer.GetWord();
                if (lexer.GetTokenType() & firstExp)
                {
                    exp();
                    while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & firstExp))
                    {
                        if (lexer.GetTokenType() & COMMA)
                            lexer.GetWord();
                        else
                            errorHandle.error(MISSING, L",", lexer.GetPreWordRow(),
                                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                        exp();
                    }
                    if (lexer.GetTokenType() & RPAREN)
                        lexer.GetWord();
                    else
                        errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                }
                else if (lexer.GetTokenType() & RPAREN)
                    lexer.GetWord();
                else if (lexer.GetTokenType() & followStatement)
                    errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
            else if (lexer.GetTokenType() & firstExp)
            {
                errorHandle.error(MISSING, L"(", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());

                exp();
                while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & firstExp))
                {
                    if (lexer.GetTokenType() & COMMA)
                        lexer.GetWord();
                    else
                        errorHandle.error(MISSING, L",", lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    exp();
                }
                if (lexer.GetTokenType() & RPAREN)
                    lexer.GetWord();
                else
                    errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
        }
        else if (lexer.GetTokenType() & LPAREN)
        {
            errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            lexer.GetWord();
            if (lexer.GetTokenType() & firstExp)
            {
                exp();
                while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & firstExp))
                {
                    if (lexer.GetTokenType() & COMMA)
                        lexer.GetWord();
                    else
                        errorHandle.error(MISSING, L",", lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    exp();
                }

                if (lexer.GetTokenType() & RPAREN)
                    lexer.GetWord();
                else
                    errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
            else if (lexer.GetTokenType() & RPAREN)
                lexer.GetWord();
            else if (lexer.GetTokenType() & followStatement)
                errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
        else if (lexer.GetTokenType() & RPAREN)
        {
            errorHandle.error(ILLEGAL_DEFINE, L"<call>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            lexer.GetWord();
        }
        else
            errorHandle.error(ILLEGAL_DEFINE, L"<call>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
    }
    else if (lexer.GetTokenType() == BEGIN_SYM) // <statement> -> <body>
        body();
    else if (lexer.GetTokenType() == READ_SYM)
    { // <statement> -> read (id{,id})
        lexer.GetWord();
        if (lexer.GetTokenType() == LPAREN)
        {
            lexer.GetWord();
            if (lexer.GetTokenType() & IDENT)
            {
                lexer.GetWord();
                while (lexer.GetTokenType() & COMMA)
                {
                    lexer.GetWord();
                    if (lexer.GetTokenType() & IDENT)
                        lexer.GetWord();
                    else if (lexer.GetTokenType() & COMMA)
                        errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    else
                    {
                        errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                        break;
                    }
                }
                if (lexer.GetTokenType() & RPAREN)
                    lexer.GetWord();
                else if (lexer.GetTokenType() & followStatement)
                    errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
            else if (lexer.GetTokenType() & RPAREN)
            {
                errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                lexer.GetWord();
            }
            else if (lexer.GetTokenType() & COMMA)
            {
                errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                while (lexer.GetTokenType() & COMMA)
                {
                    lexer.GetWord();
                    if (lexer.GetTokenType() & IDENT)
                        lexer.GetWord();
                    else if (lexer.GetTokenType() & COMMA)
                        errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());

                    else
                    {
                        errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                        break;
                    }
                }
                if (lexer.GetTokenType() & RPAREN)
                    lexer.GetWord();
                else if (lexer.GetTokenType() & followStatement)
                    errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
            else if (lexer.GetTokenType() & followStatement)
            {
                errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
        }
        else if (lexer.GetTokenType() & IDENT)
        {
            errorHandle.error(MISSING, L"(", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            lexer.GetWord();
            while (lexer.GetTokenType() & COMMA)
            {
                lexer.GetWord();
                if (lexer.GetTokenType() & IDENT)
                    lexer.GetWord();
                else if (lexer.GetTokenType() & COMMA)
                    errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());

                else
                {
                    errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    break;
                }
            }
            if (lexer.GetTokenType() & RPAREN)
                lexer.GetWord();
            else if (lexer.GetTokenType() & followStatement)
                errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
        else if (lexer.GetTokenType() & RPAREN)
        {
            errorHandle.error(ILLEGAL_DEFINE, L"<read>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            lexer.GetWord();
        }
        else if (lexer.GetTokenType() & COMMA)
        {
            errorHandle.error(MISSING, L"(<id>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            while (lexer.GetTokenType() & COMMA)
            {
                lexer.GetWord();
                if (lexer.GetTokenType() & IDENT)
                    lexer.GetWord();
                else if (lexer.GetTokenType() & COMMA)
                    errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                else
                {
                    errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    break;
                }
            }
            if (lexer.GetTokenType() & RPAREN)
                lexer.GetWord();
            else if (lexer.GetTokenType() & followStatement)
                errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
        else
            judge(0, followStatement, ILLEGAL_DEFINE, L"<read>");
    }
    else if (lexer.GetTokenType() == WRITE_SYM)
    { // <statement> -> write(<exp> {,<exp>})
        lexer.GetWord();
        if (lexer.GetTokenType() & LPAREN)
        {
            lexer.GetWord();
            if (lexer.GetTokenType() & firstExp)
            {
                exp();
                while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & firstExp))
                {
                    if (lexer.GetTokenType() & COMMA)
                        lexer.GetWord();
                    else
                        errorHandle.error(MISSING, L",", lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    exp();
                }
                if (lexer.GetTokenType() & RPAREN)
                    lexer.GetWord();
                else if (lexer.GetTokenType() & followStatement)
                    errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
            else if (lexer.GetTokenType() & RPAREN)
            {
                errorHandle.error(MISSING, L"<exp>", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                lexer.GetWord();
            }
            else if (lexer.GetTokenType() & COMMA)
            {
                errorHandle.error(MISSING, L"<exp>", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & firstExp))
                {
                    if (lexer.GetTokenType() & COMMA)
                        lexer.GetWord();
                    else
                        errorHandle.error(MISSING, L",", lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    exp();
                }
                if (lexer.GetTokenType() & RPAREN)
                    lexer.GetWord();
                else if (lexer.GetTokenType() & followStatement)
                    errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
            else if (lexer.GetTokenType() & followStatement)
            {
                errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
        }
        else if (lexer.GetTokenType() & firstExp)
        {
            errorHandle.error(MISSING, L"(", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            exp();
            while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & firstExp))
            {
                if (lexer.GetTokenType() & COMMA)
                    lexer.GetWord();
                else
                    errorHandle.error(MISSING, L",", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                exp();
            }
            if (lexer.GetTokenType() & RPAREN)
                lexer.GetWord();
            else if (lexer.GetTokenType() & followStatement)
                errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
        else if (lexer.GetTokenType() & RPAREN)
        {
            errorHandle.error(ILLEGAL_DEFINE, L"<write>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            lexer.GetWord();
        }
        else if (lexer.GetTokenType() & COMMA)
        {
            errorHandle.error(MISSING, L"(<exp>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & firstExp))
            {
                if (lexer.GetTokenType() & COMMA)
                    lexer.GetWord();
                else
                    errorHandle.error(MISSING, L",", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                exp();
            }
            if (lexer.GetTokenType() & RPAREN)
                lexer.GetWord();
            else if (lexer.GetTokenType() & followStatement)
                errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
        else
            judge(0, followStatement, ILLEGAL_DEFINE, L"<write>");
    }
    else
        // system("pause");
        //  wcout << lexer.GetTokenType() << endl;
        judge(0, followStatement, ILLEGAL_DEFINE, L"statement");
}

//<exp> → [+|-]<term>{<aop><term>}
void Parser::exp()
{
    if (lexer.GetTokenType() & firstExp)
    {
        if (lexer.GetTokenType() & (PLUS | MINUS))
            lexer.GetWord();

        if (lexer.GetTokenType() & firstTerm)
        { //<exp> → [+|-]<term>
            term();
            //<exp> → [+|-]<term>{<aop><term>}
            while (lexer.GetTokenType() & (PLUS | MINUS))
            {
                lexer.GetWord();
                term();
            }
        }
    }
    else
        // 非法的表达式开头
        judge(0, followExp, ILLEGAL_DEFINE, L"expression (invalid expression start)");
}

//<term> → <factor>{<mop><factor>}
void Parser::term()
{
    if (lexer.GetTokenType() & firstTerm)
    {             //<term> → <factor>
        factor(); // 处理第一个 factor
        while (lexer.GetTokenType() & (MULTI | DIVIS))
        { //<term> → <factor><mop>
            lexer.GetWord();
            if (lexer.GetTokenType() & firstFactor) //<term> → <factor>{<mop><factor>}
                factor();
            else if (lexer.GetTokenType() & (MULTI | DIVIS))
                // 连续操作符的错误情况
                errorHandle.error(SYNTAX_ERROR, L"<factor>",
                                  L"Two consecutive operators found. Expected a <factor> after '*' or '/'.",
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            else
                // 缺少有效的 factor
                errorHandle.error(EXPECT, L"a valid <factor> after '*' or '/'.",
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
    }
    else
    {
        // 非法的 term 开头
        int r = judge(0, followTerm, ILLEGAL_DEFINE, L"term (invalid term start)");
        if (r == 1)
            lexer.GetWord();
    }
}

//<factor>→<id>|<integer>|(<exp>)
void Parser::factor()
{
    if (lexer.GetTokenType() == IDENT)
    {
        lexer.GetWord();
    }
    else if (lexer.GetTokenType() == NUMBER)
        lexer.GetWord();
    else if (lexer.GetTokenType() == LPAREN)
    {
        lexer.GetWord();
        int r1 = judge(firstExp, RPAREN | followFactor, MISSING, L"<exp>");
        if (r1 == 1)
        {
            exp();
            if (lexer.GetTokenType() == RPAREN)
                lexer.GetWord();
            else
                errorHandle.error(MISSING_DETAILED, L"')'",
                                  L"Expected closing parenthesis ')'.",
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
    }
    else
    {
        // 其他情况处理非法的 factor
        int r = judge(0, followFactor, ILLEGAL_DEFINE, L"factor");
        if (r == 1)
            lexer.GetWord(); // 尝试恢复
        else
            errorHandle.error(SYNTAX_ERROR, L"<id>, <integer>, '('",
                              L"Expected a valid factor, such as an identifier, a number, or an expression in parentheses.",
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
    }
}

// <body> → begin <statement>{;<statement>}end
void Parser::body()
{
    if (lexer.GetTokenType() == BEGIN_SYM)
    {
        lexer.GetWord();
        statement();
        while ((lexer.GetTokenType() & SEMICOLON) || (lexer.GetTokenType() & firstStatement))
        {
            if (lexer.GetTokenType() & SEMICOLON)
                lexer.GetWord();
            else
                errorHandle.error(MISSING, L";", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            statement();
        }
        if (lexer.GetTokenType() & END_SYM)
            lexer.GetWord();
        else
            judge(0, followBody, MISSING, L"end");
    }
    else if (lexer.GetTokenType() & firstStatement)
    {
        errorHandle.error(MISSING, L"begin", lexer.GetPreWordRow(),
                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());

        while ((lexer.GetTokenType() & SEMICOLON) || (lexer.GetTokenType() & firstStatement))
        {
            if (lexer.GetTokenType() & SEMICOLON)
                lexer.GetWord();
            else
                errorHandle.error(MISSING, L";", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            statement();
        }
        if (lexer.GetTokenType() & END_SYM)
            lexer.GetWord();
        else
            judge(0, followBody, MISSING, L"end");
    }
    else if (lexer.GetTokenType() & END_SYM)
    {
        errorHandle.error(ILLEGAL_DEFINE, L"<body>", lexer.GetPreWordRow(),
                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        lexer.GetWord();
    }
    else
        judge(0, followBody, ILLEGAL_DEFINE, L"'<body>'");
}

//<lexp> → <exp> <lop> <exp>|odd <exp>
void Parser::lexp()
{
    if (lexer.GetTokenType() & firstExp)
    {
        exp();
        // 检查逻辑操作符
        if (lexer.GetTokenType() & (EQL | NEQ | LSS | LEQ | GRT | GEQ))
        {
            lexer.GetWord(); // 消耗逻辑操作符
            int r = judge(firstExp, followLexp, MISSING, L"a <exp>");
            if (r == 1)
                exp(); // 解析右侧表达式
        }
        else
            // 缺少逻辑操作符的情况
            errorHandle.error(MISSING,
                              L"Expected a logical operator (e.g., '=', '<>', '<') after the expression.",
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
    }
    else if (lexer.GetTokenType() & ODD_SYM)
    {
        lexer.GetWord();
        int r1 = judge(firstExp, followLexp, MISSING, L"a <exp>");
        if (r1 == 1)
            exp();
    }
    else
    {
        // 非法的 lexp 开头
        int r = judge(0, followLexp, ILLEGAL_DEFINE, L"lexp (invalid logical expression start)");
        if (r == 1)
        {
            lexer.GetWord(); // 尝试恢复
        }
    }
}

//<vardecl> → var <id>{,<id>};
void Parser::vardecl()
{
    if (lexer.GetTokenType() == VAR_SYM)
    {
        lexer.GetWord();
        // var <id>
        if (lexer.GetTokenType() & IDENT)
        {
            lexer.GetWord();
            while (lexer.GetTokenType() == COMMA)
            {
                lexer.GetWord();
                if (lexer.GetTokenType() & IDENT)
                    lexer.GetWord();
                else
                    errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
            if (lexer.GetTokenType() & SEMICOLON)
                lexer.GetWord();
            else
                judge(0, SEMICOLON, MISSING, L";");
        }
        else if (lexer.GetTokenType() & COMMA)
        {
            errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            while (lexer.GetTokenType() == COMMA)
            {
                lexer.GetWord();
                if (lexer.GetTokenType() & IDENT)
                    lexer.GetWord();
                else
                    errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
            if (lexer.GetTokenType() & SEMICOLON)
                lexer.GetWord();
            else
                judge(0, SEMICOLON, MISSING, L";");
        }
        else if (lexer.GetTokenType() & SEMICOLON)
        {
            errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            lexer.GetWord();
        }
        else
            judge(0, followVardecl, ILLEGAL_DEFINE, L"<var>");
    }
    else
        judge(0, followVardecl, ILLEGAL_DEFINE, L"<vardecl>");
}

//<const> → <id>:=<integer>
// const一定从condecl那边通过判断后过来
void Parser::constA()
{
    if (lexer.GetTokenType() == IDENT)
    {
        lexer.GetWord();
        //<const> → <id>:=<integer>
        if (lexer.GetTokenType() == ASSIGN)
            lexer.GetWord();
        if (lexer.GetTokenType() != NUMBER)
        {
            errorHandle.error(MISSING, L":=", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            lexer.GetWord();
        }

        lexer.GetWord();
    }
    else if (lexer.GetTokenType() == ASSIGN)
    {
        errorHandle.error(MISSING, L"<id>", lexer.GetPreWordRow(),
                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        lexer.GetWord();
        lexer.GetWord();
    }
    else if (lexer.GetTokenType() == NUMBER)
    {
        errorHandle.error(MISSING, L"<id>:=", lexer.GetPreWordRow(),
                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        lexer.GetWord();
    }
    else
        judge(0, followConst, ILLEGAL_DEFINE, L"<const>");
}

//<condecl> → const <const>{,<const>};
void Parser::condecl()
{
    if (lexer.GetTokenType() == CONST_SYM)
    {
        //<condecl> → const
        lexer.GetWord();
        if (lexer.GetTokenType() & firstConst) //<condecl> → const <const>
        {
            constA();
            while (lexer.GetTokenType() & COMMA)
            {
                lexer.GetWord();
                constA();
            }
            if (lexer.GetTokenType() & SEMICOLON)
                lexer.GetWord();
            else
                judge(0, followCondecl, MISSING, L";");
        }
        else if (lexer.GetTokenType() & COMMA)
        {
            errorHandle.error(MISSING, L"<const>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            while (lexer.GetTokenType() & COMMA)
            {
                lexer.GetWord();
                constA();
            }
            if (lexer.GetTokenType() & SEMICOLON)
                lexer.GetWord();
            else
                judge(0, followCondecl, MISSING, L";");
        }
        else if (lexer.GetTokenType() & SEMICOLON)
        {
            errorHandle.error(MISSING, L"<const>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            lexer.GetWord();
        }
        else
            judge(0, followCondecl, ILLEGAL_DEFINE, L"<condecl>");
    }
}

//<proc> → procedure <id>（[<id>{,<id>}]）;<block>{;<proc>}
void Parser::proc()
{
    if (lexer.GetTokenType() == PROC_SYM)
    {
        lexer.GetWord();
        // <proc> -> procedure <id>
        if (lexer.GetTokenType() == IDENT)
        {
            lexer.GetWord();
            if (lexer.GetTokenType() & LPAREN)
            {
                lexer.GetWord();
                if (lexer.GetTokenType() & IDENT)
                {
                    lexer.GetWord();
                    while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & IDENT))
                    {
                        if (lexer.GetTokenType() & COMMA)
                            lexer.GetWord();
                        else
                            errorHandle.error(MISSING, L"','",
                                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                        if (lexer.GetTokenType() & IDENT)
                            lexer.GetWord();
                        else
                            errorHandle.error(MISSING, L"'<id>'",
                                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    }
                }
                if (lexer.GetTokenType() & RPAREN)
                {
                    lexer.GetWord();
                    if (lexer.GetTokenType() & SEMICOLON)
                    {
                        lexer.GetWord();
                        block();
                        while (lexer.GetTokenType() & SEMICOLON)
                        {
                            lexer.GetWord();
                            proc();
                        }
                    }
                    else
                    {
                        errorHandle.error(MISSING, L"';'",
                                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                        block();
                        while (lexer.GetTokenType() & SEMICOLON)
                        {
                            lexer.GetWord();
                            proc();
                        }
                    }
                }
            }
            else if (lexer.GetTokenType() & IDENT)
            {
                errorHandle.error(MISSING, L"'('",
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                lexer.GetWord();
                while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & IDENT))
                {
                    if (lexer.GetTokenType() & COMMA)
                        lexer.GetWord();
                    else
                        errorHandle.error(MISSING, L"','",
                                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    if (lexer.GetTokenType() & IDENT)
                        lexer.GetWord();
                    else
                        errorHandle.error(MISSING, L"'<id>'",
                                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                }
                if (lexer.GetTokenType() & RPAREN)
                {
                    lexer.GetWord();
                    if (lexer.GetTokenType() & SEMICOLON)
                    {
                        lexer.GetWord();
                        block();
                        while (lexer.GetTokenType() & SEMICOLON)
                        {
                            lexer.GetWord();
                            proc();
                        }
                    }
                    else
                    {
                        errorHandle.error(MISSING, L"';'",
                                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                        block();
                        while (lexer.GetTokenType() & SEMICOLON)
                        {
                            lexer.GetWord();
                            proc();
                        }
                    }
                }
            }
            else if (lexer.GetTokenType() & RPAREN)
            {
                errorHandle.error(MISSING, L"'('",
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                lexer.GetWord();
                if (lexer.GetTokenType() & SEMICOLON)
                {
                    lexer.GetWord();
                    block();
                    while (lexer.GetTokenType() & SEMICOLON)
                    {
                        lexer.GetWord();
                        proc();
                    }
                }
                else
                {
                    errorHandle.error(MISSING, L"';'",
                                      lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    block();
                    while (lexer.GetTokenType() & SEMICOLON)
                    {
                        lexer.GetWord();
                        proc();
                    }
                }
            }
        }
        else if (lexer.GetTokenType() & LPAREN)
        {
            errorHandle.error(MISSING, L"'<id>'",
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());

            lexer.GetWord();
            if (lexer.GetTokenType() & IDENT)
            {
                lexer.GetWord();
                while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & IDENT))
                {
                    if (lexer.GetTokenType() & COMMA)
                        lexer.GetWord();
                    else
                        errorHandle.error(MISSING, L"','",
                                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    if (lexer.GetTokenType() & IDENT)
                        lexer.GetWord();
                    else
                        errorHandle.error(MISSING, L"'<id>'",
                                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                }
            }
            if (lexer.GetTokenType() & RPAREN)
            {
                lexer.GetWord();
                if (lexer.GetTokenType() & SEMICOLON)
                {
                    lexer.GetWord();
                    block();
                    while (lexer.GetTokenType() & SEMICOLON)
                    {
                        lexer.GetWord();
                        proc();
                    }
                }
                else
                {
                    errorHandle.error(MISSING, L"';'",
                                      lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    block();
                    while (lexer.GetTokenType() & SEMICOLON)
                    {
                        lexer.GetWord();
                        proc();
                    }
                }
            }
        }
    }
    else
        judge(0, followProc, ILLEGAL_DEFINE, L"procedure");
}

//<block> → [<condecl>][<vardecl>][<proc>]<body>
void Parser::block()
{
    int r = judge(firstBlock, followBlock, MISSING, L"body");
    if (r == 1)
    {
        //<block> → <condecl>
        if (lexer.GetTokenType() & firstCondecl)
            condecl();

        //<block> → [<condecl>]<vardecl>
        if (lexer.GetTokenType() & firstVardecl)
            vardecl();

        //<block> → [<condecl>][<vardecl>]<proc>
        if (lexer.GetTokenType() & firstProc)
            proc();

        // wcout << lexer.GetStrToken() << endl;
        //<block> → [<condecl>][<vardecl>][<proc>]<body>
        body();
    }
}

//<prog> → program <id>；<block>
void Parser::prog()
{
    int r = judge(PROGM_SYM, IDENT | SEMICOLON | firstBlock, MISSING, L"program");
    if (r == 1)
        lexer.GetWord();

    //<prog> → program <id>
    if (lexer.GetTokenType() == IDENT)
    {
        lexer.GetWord();
        if (lexer.GetTokenType() == SEMICOLON)
        {
            lexer.GetWord();
            block(); // 进入<block>

            if (lexer.GetCh() != L'\0' && lexer.GetCh() != L'#')
                errorHandle.error(ILLEGAL_WORD, (L"'" + lexer.GetStrToken() + L"'").c_str(),
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            return;
        }
        else
        {
            errorHandle.error(MISSING, L";",
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            block();
            if (lexer.GetCh() != L'\0' && lexer.GetCh() != L'#')
                errorHandle.error(ILLEGAL_WORD, (L"'" + lexer.GetStrToken() + L"'").c_str(),
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            return;
        }
    } //<prog> → program illegal_word

    if (lexer.GetTokenType() == SEMICOLON)
    {
        // ERROR
        errorHandle.error(MISSING, L"program name",
                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());

        // TODO
        lexer.GetWord();
        block(); // 进入<block>
        if (lexer.GetCh() != '\0' && lexer.GetCh() != L'#')
            errorHandle.error(ILLEGAL_WORD, (L"'" + lexer.GetStrToken() + L"'").c_str(),
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        return;
    }

    if (lexer.GetTokenType() & firstBlock)
    {
        // ERROR
        errorHandle.error(EXPECT_STH_FIND_ANTH, L"id", (L"'" + lexer.GetStrToken() + L"'").c_str(),
                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        block(); // 进入<block>
        if (lexer.GetCh() != '\0' && lexer.GetCh() != L'#')
            errorHandle.error(ILLEGAL_WORD, (L"'" + lexer.GetStrToken() + L"'").c_str(),
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        return;
    }

    return;
}

void Parser::analyze()
{
    lexer.GetWord();
    prog();
    // system("pause");
    errorHandle.over();
    // system("pause");
}
