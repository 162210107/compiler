#include <parser.hpp>
Parser parser;

void Parser::reportError(unsigned int errorType, const wchar_t* expected, const wchar_t* context) {
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
        }else
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
            if (lexer.GetCh() == L'\0'){
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
        }else
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
        judge(ASSIGN, firstExp | followStatement, MISSING, L":=");
        if (lexer.GetTokenType() == ASSIGN)
        {
            lexer.GetWord();
            int r = judge(firstExp, followStatement, MISSING, L"<exp>");
            if (r == 1)
            {
                exp();
                return;
            }
        }
        else if (lexer.GetTokenType() & firstExp)
        {
            exp();
            return;
        }
    }
    else if (lexer.GetTokenType() == IF_SYM)
    { // <statement> -> if <lexp> then <statement> [else <statement>]
        lexer.GetWord();
        int r = judge(firstLexp, THEN_SYM | followStatement, MISSING, L"<lexp>");
        if (r == 1)
        {
            lexp();
            r = judge(THEN_SYM, firstStatement | followStatement, MISSING, L"then");
            if (r == 1)
                goto Entry8;
            else if (lexer.GetTokenType() & firstStatement)
            {
                statement();
            Entry9:
                if (lexer.GetTokenType() == ELSE_SYM)
                {
                    lexer.GetWord();
                    r = judge(firstStatement, followStatement, MISSING, L"statement");
                    if (r == 1)
                    {
                        statement();
                        goto Entry9;
                    }
                    else
                        return;
                }
            }
            else
                return;
        }
        else if (lexer.GetTokenType() & THEN_SYM)
        {
        Entry8:
            lexer.GetWord();

            r = judge(firstStatement, followStatement, MISSING, L"then");
            if (r == 1)
            {
                statement();
                goto Entry9;
            }
            else
                return;
        }
    }
    else if (lexer.GetTokenType() == WHILE_SYM)
    { // <statement> -> while <lexp> do <statement>
        lexer.GetWord();
        int r = judge(firstLexp, DO_SYM | followStatement, MISSING, L"<lexp>");
        if (r == 1)
        {
            lexp();
            judge(DO_SYM, firstStatement | followStatement, MISSING, L"do");
            if (lexer.GetTokenType() == DO_SYM)
            {
                lexer.GetWord();
                judge(firstStatement, followStatement, MISSING, L"statement");
                if (lexer.GetTokenType() & firstStatement)
                    statement();
            }
            else if (lexer.GetTokenType() & firstStatement)
                statement();
            else
                return;
        }
    }
    else if (lexer.GetTokenType() == CALL_SYM)
    { // <statement> -> call id ([{<exp>{,<exp>}])
        lexer.GetWord();
        judge(IDENT, LPAREN | followStatement, MISSING, L"<id>");
        // <statement> -> call id
        if (lexer.GetTokenType() == IDENT)
        {
            lexer.GetWord();
        }
        else if (lexer.GetTokenType() & followStatement)
            return;

        // <statement> -> call id (
        if (lexer.GetTokenType() == LPAREN)
            lexer.GetWord();
        else
        {
            int r2 = judge(0, firstExp | RPAREN, MISSING, L"'('");
            if (r2 == 1)
                lexer.GetWord();
        }
        // <statement> -> call id ([{<exp>
        if (lexer.GetTokenType() & firstExp)
        {
            exp();
            size_t i = 1;
            // <statement> -> call id ([{<exp>{,<exp>}]
            while (lexer.GetTokenType() == COMMA)
            {
                lexer.GetWord();
                if (lexer.GetTokenType() & firstExp)
                    exp();
                else
                {
                    int r3 = judge(0, firstExp, REDUNDENT, L"','");
                    if (r3 == 1)
                        lexer.GetWord();
                    exp();
                }
            }
        }
        // <statement> -> call id ([{<exp>{,<exp>}])
        if (lexer.GetTokenType() == RPAREN)
            lexer.GetWord();
    }
    else if (lexer.GetTokenType() == BEGIN_SYM)
    { // <statement> -> <body>
        int r = judge(firstBody, followStatement, MISSING, L"<body>");
        if (r == 1)
            body();
    }
    else if (lexer.GetTokenType() == READ_SYM)
    { // <statement> -> read (id{,id})
        lexer.GetWord();
        if (lexer.GetTokenType() == LPAREN)
            lexer.GetWord();
        else
        {
            int r = judge(0, IDENT, MISSING, L"'('");
            if (r == 1)
                lexer.GetWord();
        }
        // <statement> -> read (id
        if (lexer.GetTokenType() == IDENT)
        {
            lexer.GetWord();
        }
        
        else
        {
            int r = judge(0, COMMA | RPAREN, EXPECT_STH_FIND_ANTH, L"identifier", (L"'" + lexer.GetStrToken() + L"'").c_str());
            if (r == 1)
                lexer.GetWord();
        }
        // <statement> -> read (id{,
        while (lexer.GetTokenType() == COMMA)
        {
            lexer.GetWord();
            if (lexer.GetTokenType() == IDENT)
            {
                lexer.GetWord();
            }
            else
            {
                int r = judge(0, IDENT, REDUNDENT, lexer.GetStrToken().c_str());
                if (r == 1)
                    lexer.GetWord();
            }
        }
        if (lexer.GetTokenType() == RPAREN)
            lexer.GetWord();
        else
        {
            int r = judge(0, followStatement, MISSING, L"')'");
            if (r == 1)
                lexer.GetWord();
        }
    }
    else if (lexer.GetTokenType() == WRITE_SYM)
    { // <statement> -> write(<exp> {,<exp>})
        lexer.GetWord();
        // <statement> -> write(
        if (lexer.GetTokenType() == LPAREN)
            lexer.GetWord();
        else
        {
            int r = judge(0, firstExp, MISSING, L"'('");
            if (r == 1)
                lexer.GetWord();
        }
        // <statement> -> write(<exp>
        exp();
        // <statement> -> write(<exp> {,<exp>}
        while (lexer.GetTokenType() == COMMA)
        {
            lexer.GetWord();
            if (lexer.GetTokenType() == RPAREN)
                errorHandle.error(REDUNDENT, L"','",
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            else
                exp();
        }
        // <statement> -> write(<exp> {,<exp>})
        if (lexer.GetTokenType() == RPAREN)
            lexer.GetWord();
        else
        {
            int r = judge(0, followStatement, MISSING, L"')'");
            if (r == 1)
                lexer.GetWord();
        }
    }
    else
    {
        int r = judge(0, followStatement, ILLEGAL_DEFINE, L"statement");
        if (r == 1)
            lexer.GetWord();
    }
}

//<exp> → [+|-]<term>{<aop><term>}
void Parser::exp()
{
    if (lexer.GetTokenType() & firstExp)
    {
        if (lexer.GetTokenType() & (PLUS | MINUS))
            lexer.GetWord();

        int r = judge(firstTerm, followExp, MISSING, L"term (valid term expected in expression)");
        if (r == 1)
        { //<exp> → [+|-]<term>
            term();

            //<exp> → [+|-]<term>{<aop><term>}
            while (lexer.GetTokenType() & (PLUS | MINUS))
            {
                lexer.GetWord();
                if (lexer.GetTokenType() & firstTerm)
                    term();
                else if(lexer.GetTokenType() & (PLUS | MINUS))
                    // 在发现 '+' 或 '-' 之后未找到有效的 <term>
                    errorHandle.error(EXPECT, L"a valid term after '+' or '-' in expression.",
                                      lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
        }
    }
    else{
       // 非法的表达式开头
        int r = judge(0, followExp, ILLEGAL_DEFINE, L"expression (invalid expression start)");
        if (r == 1)
            lexer.GetWord();
    }
}

//<term> → <factor>{<mop><factor>}
void Parser::term()
{
    if (lexer.GetTokenType() & firstTerm)
    { //<term> → <factor>
        factor();// 处理第一个 factor
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
    else{
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
void Parser::body() {
    int r = judge(BEGIN_SYM|firstStatement ,followBody, MISSING, L"'begin'");
    if (lexer.GetTokenType() == BEGIN_SYM) lexer.GetWord();
    else if(r==-1||r==2) return;
    
    statement();

    while (lexer.GetTokenType() & SEMICOLON) {
        lexer.GetWord();  
        statement();
        if (lexer.GetTokenType() == END_SYM) {
            break;
        }
    }

    int r1 = judge(END_SYM, followBody, MISSING, L"'end'");
    if (r1 == 1) lexer.GetWord();
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
        if (lexer.GetTokenType() == IDENT)
        {
            lexer.GetWord();
        }
        else
        {
            int r = judge(0, COMMA, MISSING, L"identifier");
            if (r == 1)
                lexer.GetWord();
        }
        // var <id>{,<id>}
        while (lexer.GetTokenType() == COMMA)
        {
            lexer.GetWord();
            if (lexer.GetTokenType() == IDENT)
            {
                lexer.GetWord();
            }
            else {
                reportError(MISSING, L"identifier", L"Expected a variable identifier after ','.");
            }
        }
        // var <id>{,<id>};
        if (lexer.GetTokenType() == SEMICOLON)
            lexer.GetWord();
        else
        {
            int r = judge(0, followVardecl, MISSING, L"';'");
            if (r == 1)
                lexer.GetWord();
        }
    }
    else
    {
        int r = judge(0, followVardecl, ILLEGAL_DEFINE, L"<vardecl>");
        if (r == 1)
            lexer.GetWord();
    }
}

//<const> → <id>:=<integer>
// const一定从condecl那边通过判断后过来
void Parser::constA()
{
    //<const> → <id>
    lexer.GetWord();
    int r = judge(ASSIGN, NUMBER | followConst, MISSING, L":=");

    //<const> → <id>:=
    if (r == 1)
    {
        lexer.GetWord();
        r = judge(NUMBER, followConst, MISSING, L"integer");

        //<const> → <id>:=<integer>
        if (r == 1)
        {
            lexer.GetWord();
        }
    }
    else if (lexer.GetTokenType() == NUMBER)
    { //<const> → <id> <integer>
            lexer.GetWord();
    } // 到const的follow集合
}

//<condecl> → const <const>{,<const>};
void Parser::condecl()
{
    if (lexer.GetTokenType() == CONST_SYM)
    {
        //<condecl> → const
        lexer.GetWord();

        if (lexer.GetTokenType() == firstConst) //<condecl> → const <const>
            constA();
        else
            reportError(MISSING, L"constant declaration", L"Expected a constant after 'const'.");

        while (lexer.GetTokenType() == COMMA)
        {
            lexer.GetWord();
            if (lexer.GetTokenType() == firstConst)
                constA();
            else if (lexer.GetTokenType() == COMMA)
                reportError(REDUNDENT, L"','", L"Unexpected ',' without a following constant.");
        }

        if (lexer.GetTokenType() == SEMICOLON)
            lexer.GetWord();
        else
        {
            int r = judge(SEMICOLON, followCondecl, EXPECT_STH_FIND_ANTH, L";", lexer.GetStrToken().c_str());
            if (r == 1)
                lexer.GetWord();
        }
    }
    else
    {
        int r = judge(0, followCondecl, ILLEGAL_DEFINE, L"condecl");
        if (r == 1)
            lexer.GetWord();
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
        }
        else
        {
            int r = judge(0, LPAREN, EXPECT_STH_FIND_ANTH, L"identifier", (L"'" + lexer.GetStrToken() + L"'").c_str());
            if (r == 1)
                lexer.GetWord();
        }
        // <proc> -> procedure id (
        if (lexer.GetTokenType() == LPAREN)
        {
            lexer.GetWord();
        }
        else
        {
            int r = judge(0, IDENT | RPAREN, MISSING, L"'('");
            if (r == 1)
                lexer.GetWord();
        }
        // <proc> -> procedure id ([id {,id}]
        // 分析至形参列表
        if (lexer.GetTokenType() == IDENT)
        {
            lexer.GetWord();
            while (lexer.GetTokenType() == COMMA)
            {
                lexer.GetWord();
                if (lexer.GetTokenType() == IDENT)
                {
                    lexer.GetWord();
                }
                else
                {
                    errorHandle.error(REDUNDENT, L"','",
                                      lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                }
            }
        }
        // <proc> -> procedure id ([id {,id}])
        if (lexer.GetTokenType() == RPAREN)
        {
            lexer.GetWord();
        }
        else
        {
            int r = judge(0, SEMICOLON, MISSING, L"')'");
            if (r == 1)
                lexer.GetWord();
        }
        // <proc> -> procedure id ([id {,id}]);
        if (lexer.GetTokenType() == SEMICOLON)
        {
            lexer.GetWord();
        }
        else
        {
            int r = judge(0, firstBlock, MISSING, L"';'");
            if (r == 1)
                lexer.GetWord();
        }
        // <proc> -> procedure id ([id {,id}]);<block> {;<proc>}

        if (lexer.GetTokenType() & firstBlock)
        {
            block();
            // 当前过程结束，开始分析下面的过程
            while (lexer.GetTokenType() == SEMICOLON)
            {
                lexer.GetWord();
                // FIRST(proc)
                if (lexer.GetTokenType() == PROC_SYM)
                {
                    proc();
                }
                else
                {
                    errorHandle.error(REDUNDENT, L"';'",
                                      lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                }
            }
        }
        else
        {
            int r = judge(0, followBlock, ILLEGAL_DEFINE, L"<block>");
            if (r == 1)
                lexer.GetWord();
        }
    }
    else
    {
        int r = judge(0, followProc, ILLEGAL_DEFINE, L"procedure");
        if (r == 1)
            lexer.GetWord();
    }
}

//<block> → [<condecl>][<vardecl>][<proc>]<body>
void Parser::block()
{
    int r=judge(firstBlock,followBlock,MISSING,L"body");
    if(r==1){
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
    int r = judge(firstBody, followBlock, MISSING, L"'begin' (start of a block body)");
    if (r == 1)
        body();
    }else{
        return;
    }
}

//<prog> → program <id>；<block>
void Parser::prog()
{
    int r=judge(PROGM_SYM, IDENT|SEMICOLON|firstBlock, MISSING, L"program");
    if(r==1)
        lexer.GetWord();

    //<prog> → program <id>
    if (lexer.GetTokenType() == IDENT)
    {
        lexer.GetWord();
        if (lexer.GetTokenType() == SEMICOLON)
        {
            lexer.GetWord();
            block(); // 进入<block>

            if(lexer.GetCh()!=L'\0'&&lexer.GetCh()!=L'#')
                errorHandle.error(ILLEGAL_WORD, (L"'" + lexer.GetStrToken()+ L"'").c_str(),
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
        if(lexer.GetCh()!='\0'&&lexer.GetCh()!=L'#')
                errorHandle.error(ILLEGAL_WORD, (L"'" + lexer.GetStrToken() + L"'").c_str(),
                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        return;
    }

    if(lexer.GetTokenType() & firstBlock)
    {
            // ERROR
        errorHandle.error(EXPECT_STH_FIND_ANTH, L"id", (L"'" + lexer.GetStrToken() + L"'").c_str(),
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        block(); // 进入<block>
        if(lexer.GetCh()!='\0'&&lexer.GetCh()!=L'#')
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
