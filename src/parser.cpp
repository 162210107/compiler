#include <parser.hpp>
Parser parser;

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
                errorHandle.over();
            lexer.GetWord(); // 继续词法分析
        }
        if (lexer.GetTokenType() & s1)
        {
            return 1;
        }
        else
        {
            // 已经获取了下一个匹配的first，所以无需再读
            return -1;
        }
    }
    else
    {
        return 1;
    }
}

// 输出：1是匹配到s1,-1匹配到s2
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
                errorHandle.over();
            lexer.GetWord(); // 继续词法分析
        }

        if (lexer.GetTokenType() & s1)
            return 1;
        else
            // 已经获取了下一个匹配的first，所以无需再读
            return -1;
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
        // 查找当前变量是否在符号表中
        int pos = symTable.SearchInfo(lexer.GetStrToken(), Category::VAR);
        if (pos == -1)
            errorHandle.error(UNDECLARED_IDENT, lexer.GetStrToken().c_str(),
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());

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
            // 查找过程的符号名
            int pos = symTable.SearchInfo(lexer.GetStrToken(), Category::VAR);
            // 未查找到过程名
            if (pos == -1)
                errorHandle.error(UNDECLARED_PROC, lexer.GetStrToken().c_str(),
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
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
            // 查找变量符号
            int pos = symTable.SearchInfo(lexer.GetStrToken(), Category::VAR);
            if (pos == -1)
                errorHandle.error(UNDECLARED_IDENT, lexer.GetStrToken().c_str(),
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
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
                int pos = symTable.SearchInfo(lexer.GetStrToken(), Category::VAR);
                // 未查找到符号
                if (pos == -1)
                    errorHandle.error(UNDECLARED_IDENT, lexer.GetStrToken().c_str(),
                                      lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
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
    if (lexer.GetTokenType() & (PLUS | MINUS))
    {
        lexer.GetWord();
    }

    int r = judge(firstTerm, followExp, MISSING, L"term");
    if (r == 1)
    { //<exp> → [+|-]<term>
        term();

    Entry5: //<exp> → [+|-]<term>{<aop><term>}
        if (lexer.GetTokenType() & (PLUS | MINUS))
        {
            int r1 = judge(firstTerm, followExp, MISSING, L"a <aop>");
            if (r1 == 1)
            {
                term();
                goto Entry5;
            }
        }
    }
    else
        return;
}

//<term> → <factor>{<mop><factor>}
void Parser::term()
{
    int r = judge(firstFactor, followTerm, MISSING, L"a <factor>");
    if (r == 1)
    { //<term> → <factor>
        factor();
    Entry6:
        if (lexer.GetTokenType() & (MULTI | DIVIS))
        { //<term> → <factor><mop>
            lexer.GetWord();
            if (lexer.GetTokenType() & firstFactor)
            { //<term> → <factor>{<mop><factor>}
                factor();
                goto Entry6;
            }
            else
                errorHandle.error(EXPECT, L"<factor>",
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
    }
    else
        return;
}

//<factor>→<id>|<integer>|(<exp>)
void Parser::factor()
{
    int r = judge(IDENT | NUMBER | LPAREN, followFactor, EXPECT_STH_FIND_ANTH, L"'<id>,<integer>,('", lexer.GetStrToken().c_str());
    if (lexer.GetTokenType() == IDENT)
    {
        // 查找变量符号
        int pos = symTable.SearchInfo(lexer.GetStrToken(), Category::CST);
        VarInfo *cur_info = nullptr;
        if (pos == -1)
            errorHandle.error(UNDECLARED_IDENT, lexer.GetStrToken().c_str(),
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        // 若为常量，直接获取其符号表中的右值
        // 用临时变量记录当前查到的信息
        lexer.GetWord();
    }
    else if (lexer.GetTokenType() == NUMBER)
    {
        lexer.GetWord();
    }
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
                errorHandle.error(MISSING, L")",
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
    }
    else
        return;
}

//<body> → begin <statement>{;<statement>}end
void Parser::body()
{
    // 判断是否存在begin,是否仅缺少begin
    int r = judge(BEGIN_SYM, firstStatement | followBody, MISSING, L"begin");

    if (r == 1)
    {
        lexer.GetWord();
    Entry7:
        int r1 = judge(firstStatement, END_SYM | followBody, MISSING, L"statement");
        if (r1 == 1)
        {
            statement();
            int r2 = judge(SEMICOLON | END_SYM, followBody, MISSING, L"end");
            if (lexer.GetTokenType() == SEMICOLON)
            {
                lexer.GetWord();
                goto Entry7;
            }
            else if (lexer.GetTokenType() == END_SYM)
            {
                lexer.GetWord();
                return;
            }
            else
                return;
        }
        else if (lexer.GetTokenType() == END_SYM)
        {
            lexer.GetWord();
            return;
        }
        else
            return;
    }
    else if (lexer.GetTokenType() & firstStatement)
    {
        goto Entry7;
    }
    else
        return;
}

//<lexp> → <exp> <lop> <exp>|odd <exp>
void Parser::lexp()
{
    int r = judge(firstExp | ODD_SYM, followLexp, MISSING, L"a <exp> or odd");

    if (lexer.GetTokenType() & firstExp)
    {
        exp();
        int r1 = judge(EQL | NEQ | LSS | LEQ | GRT | GEQ, firstExp | followLexp, MISSING, L"a <lop>");
        if (r1 == 1)
        {
            lexer.GetWord();
            exp();
        }
        else if (lexer.GetTokenType() & firstExp)
        {
            exp();
        }
        else
            return;
    }
    else if (lexer.GetTokenType() & ODD_SYM)
    {
        lexer.GetWord();
        int r1 = judge(firstExp, followLexp, MISSING, L"a <exp>");
        if (r1 == 1)
            exp();
    }
    else
        return;
}

//<vardecl> → var <id>{,<id>};
void Parser::vardecl()
{
    int r = judge(firstVardecl, IDENT | followVardecl, EXPECT_STH_FIND_ANTH, L"var", lexer.GetStrToken().c_str());

    if (r == 1)
    { //<vardecl> → var
        lexer.GetWord();
    Entry4:
        int r1 = judge(IDENT, SEMICOLON | COMMA | followVardecl, MISSING, L"<id>");
        if (r1 == 1)
        { //<vardecl> → var <id>
            symTable.InsertToTable(lexer.GetStrToken(), Category::VAR);
            lexer.GetWord();
            goto Entry3;
        }
        else if (lexer.GetTokenType() == SEMICOLON)
        { //<vardecl> → var ;
            lexer.GetWord();
            return;
        }
        else if (lexer.GetTokenType() == COMMA)
        { //<vardecl> → var ,
            lexer.GetWord();
            goto Entry4;
        }
        else
            return;
    }
    else if (lexer.GetTokenType() == IDENT)
    { //<vardecl> → <id>
        int curAddr = symTable.InsertToTable(lexer.GetStrToken(), Category::VAR);
        lexer.GetWord();
        goto Entry3;
    }
    else
        return;

Entry3:
    //<vardecl> → var <id>
    int r2 = judge(SEMICOLON | COMMA, IDENT | followVardecl, MISSING, L",");
    if (lexer.GetTokenType() == SEMICOLON)
    { //<vardecl> → var <id> ;
        lexer.GetWord();
        return;
    }
    else if (lexer.GetTokenType() == COMMA)
    {
        lexer.GetWord();
        goto Entry4;
    }
    else if (lexer.GetTokenType() == IDENT)
    {
        symTable.InsertToTable(lexer.GetStrToken(), Category::VAR);
        lexer.GetWord();
        goto Entry3;
    }
    else
        return;
}

//<const> → <id>:=<integer>
// const一定从condecl那边通过判断后过来
// 我只负责多往后看一个！以及整体观念
void Parser::constA()
{
    // 插入符号表。此时value为0
    int curAddr = symTable.InsertToTable(lexer.GetStrToken(), Category::CST);

    //<const> → <id>
    lexer.GetWord();
    int r = judge(ASSIGN, NUMBER | followConst, MISSING, L":=");

    //<const> → <id>:=
    if (r == 1)
    {
        lexer.GetWord();
        r = judge(NUMBER, followConst, MISSING, L"integer");

        //<const> → <id>:=<integer>
        if (r == 1 && curAddr != -1)
        {
            // 存进去TODO
            SymTableItem item = symTable.GetTable(curAddr);
            item.info->SetValue(lexer.GetStrToken());
            lexer.GetWord();
        }
        else if (r == -1)
        { //<const> → <id>:=  读到follow
        }
        else if (r == 1 && curAddr == -1)
        { //<const> → <id>:=<integer>,但是重复存在
            lexer.GetWord();
        }
    }
    else if (lexer.GetTokenType() == NUMBER)
    { //<const> → <id> <integer>
        if (curAddr != -1)
        {
            SymTableItem item = symTable.GetTable(curAddr);
            item.info->SetValue(lexer.GetStrToken());
            lexer.GetWord();
        }
        else
        { // 已经重复的元素
            lexer.GetWord();
        }
    } // 到const的follow集合
}

//<condecl> → const <const>{,<const>};
// 只有从block过来，并且block那边过来时已经有验证
// 匹配到；后多读一个，供后续用
// 本身过来就有一个token
void Parser::condecl()
{
    //<condecl> → const
    lexer.GetWord();

Entry1:
    int r = judge(firstConst, followConst | followCondecl, MISSING, L"<const>");
    if (lexer.GetTokenType() == SEMICOLON)
    { //<condecl> → const {,} ;
        lexer.GetWord();
        return;
    }
    if (r == 1)
    { //<condecl> → const <const>
        constA();
        goto Entry2;
    }
    else if (lexer.GetTokenType() == COMMA)
    { //<condecl> → const {,}
        lexer.GetWord();
        goto Entry1;
    }
    else
    {
        return;
    }

Entry2:
    int r1 = judge(followConst, firstConst | followCondecl, MISSING, L",/;");
    if (lexer.GetTokenType() == COMMA)
    { //<condecl> → const <const> ,
        lexer.GetWord();
        goto Entry1;
    }
    else if (lexer.GetTokenType() == SEMICOLON)
    { //<condecl> → const <const>;
        lexer.GetWord();
        return;
    }
    else if (lexer.GetTokenType() == IDENT)
    { //<condecl> → const <const> <const>
        constA();
        goto Entry2;
    }
    else
        return;
}

//<proc> → procedure <id>（[<id>{,<id>}]）;<block>{;<proc>}
void Parser::proc()
{
    if (lexer.GetTokenType() == PROC_SYM)
    {
        lexer.GetWord();
        ProcInfo *cur_info = nullptr;
        // <proc> -> procedure id
        if (lexer.GetTokenType() == IDENT)
        {
            // 将过程名登入符号表
            symTable.MkTable();
            int cur_proc = symTable.InsertToTable(lexer.GetStrToken(), Category::PROCE);
            if (cur_proc != -1)
            {
                cur_info = (ProcInfo *)symTable.table[cur_proc].info;
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
                // 层级增加，display表扩张
                symTable.display.push_back(0);
                symTable.level++;
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
                // 将过程的形参登入符号表，并与相应的过程绑定
                int form_var = symTable.InsertToTable(lexer.GetStrToken(), Category::FORM);
                if (cur_info)
                    cur_info->formVarList.push_back(form_var);
                lexer.GetWord();
                while (lexer.GetTokenType() == COMMA)
                {
                    lexer.GetWord();
                    if (lexer.GetTokenType() == IDENT)
                    {
                        // 将过程的形参登入符号表，并与相应的过程绑定
                        int form_var = symTable.InsertToTable(lexer.GetStrToken(), Category::FORM);
                        if (cur_info)
                            cur_info->formVarList.push_back(form_var);
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
                // 层级减少，display表弹出
                symTable.display.pop_back();
                symTable.level--;
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
                        errorHandle.error(REDUNDENT, L"';'",
                                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                }
            }
            else
            {
                int r = judge(0, followBlock, ILLEGAL_DEFINE, L"block");
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
}

//<block> → [<condecl>][<vardecl>][<proc>]<body>
void Parser::block()
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

    //<block> → [<condecl>][<vardecl>][<proc>]<body>
    int r = judge(firstBody, followBlock, EXPECT_STH_FIND_ANTH, L"begin", lexer.GetStrToken().c_str());
    if (r == 1)
        body();
    else
        return;
}

//<prog> → program <id>；<block>
void Parser::prog()
{
    judge(PROGM_SYM, 0, MISSING, L"program");

    lexer.GetWord();
    //<prog> → program <id>
    if (lexer.GetTokenType() == IDENT)
    {
        symTable.MkTable();
        symTable.InsertToTable(lexer.GetStrToken(), Category::PROCE);
    } //<prog> → program ;
    else if (lexer.GetTokenType() == SEMICOLON)
    {
        // ERROR
        errorHandle.error(MISSING, L"program name",
                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());

        // TODO

        lexer.GetWord();
        block(); // 进入<block>
        return;
    } //<prog> → program illegal_word
    else
    {
        ILLEGAL_WORD, (L"'" + lexer.GetStrToken() + L"'"),
            // ERROR
            errorHandle.error(EXPECT_STH_FIND_ANTH, L"id", (L"'" + lexer.GetStrToken() + L"'").c_str(),
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
    }

    lexer.GetWord();
    //<prog> → program <id> ; <block>
    int r = judge(SEMICOLON, firstBlock, MISSING, L";");
    if (r == 1)
        lexer.GetWord();
    // TODO

    block();
}
