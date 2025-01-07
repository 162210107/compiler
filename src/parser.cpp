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
        int pos = symTable.SearchInfo(lexer.GetStrToken(), Category::VAR);
        VarInfo *cur_info = nullptr;
        if (pos == -1)
            errorHandle.error(UNDECLARED_IDENT, lexer.GetStrToken().c_str(),
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        else
            cur_info = (VarInfo *)symTable.table[pos].info;
        lexer.GetWord();
        if (lexer.GetTokenType() == ASSIGN)
        {
            if (cur_info && cur_info->cat == Category::CST)
                errorHandle.error(ILLEGAL_RVALUE_ASSIGN, lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
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
            errorHandle.error(ILLEGAL_DEFINE, L"<ident>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        if (cur_info)
            // 赋值的P代码，当前栈顶为计算出的表达式
            pcodelist.emit(store, cur_info->level, cur_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + cur_info->level + 1);
    }
    else if (lexer.GetTokenType() & IF_SYM)
    { // <statement> -> if <lexp> then <statement> [else <statement>]
        lexer.GetWord();
        lexp();
        int entry_jpc = -1, entry_jmp = -1;
        // system("pause");
        // wcout << lexer.GetTokenType() << endl;
        if (lexer.GetTokenType() & THEN_SYM)
        {
            entry_jpc = pcodelist.emit(jpc, 0, 0);
            // system("pause");
            // wcout << (lexer.GetTokenType()&THEN_SYM)<<L"now" << endl;
            lexer.GetWord();
            statement();
            if (lexer.GetTokenType() & ELSE_SYM)
            {
                entry_jmp = pcodelist.emit(jmp, 0, 0);
                lexer.GetWord();
                // 将else入口地址回填至jpc
                pcodelist.backpatch(entry_jpc, pcodelist.code_list.size());
                statement();
                // 有else，则将if外入口地址回填至jmp
                pcodelist.backpatch(entry_jmp, pcodelist.code_list.size());
            }
            else
                // 没有else，则将if外入口地址回填至jpc
                pcodelist.backpatch(entry_jpc, pcodelist.code_list.size());
        }
        else if (lexer.GetTokenType() & firstStatement)
        {
            entry_jpc = pcodelist.emit(jpc, 0, 0);
            // system("pause");
            errorHandle.error(MISSING, L"then", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            statement();
            if (lexer.GetTokenType() & ELSE_SYM)
            {
                entry_jmp = pcodelist.emit(jmp, 0, 0);
                lexer.GetWord();
                // 将else入口地址回填至jpc
                pcodelist.backpatch(entry_jpc, pcodelist.code_list.size());
                statement();
                // 有else，则将if外入口地址回填至jmp
                pcodelist.backpatch(entry_jmp, pcodelist.code_list.size());
            }
            else
                // 没有else，则将if外入口地址回填至jpc
                pcodelist.backpatch(entry_jpc, pcodelist.code_list.size());
        }
        else if (lexer.GetTokenType() & ELSE_SYM)
        {
            entry_jmp = pcodelist.emit(jmp, 0, 0);
            // system("pause");
            // wcout << lexer.GetTokenType() << endl;
            lexer.GetWord();
            statement();
            pcodelist.backpatch(entry_jmp, pcodelist.code_list.size());
        }
        else
            errorHandle.error(ILLEGAL_DEFINE, L"<if>", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
    }
    else if (lexer.GetTokenType() == WHILE_SYM)
    { // <statement> -> while <lexp> do <statement>
        lexer.GetWord();
        // FIRST(lexp)
        size_t condition = pcodelist.code_list.size();
        lexp();
        // 当前栈顶为条件表达式的布尔值
        // 条件为假跳转，待回填循环出口地址
        size_t loop = pcodelist.emit(jpc, 0, 0);
        if (lexer.GetTokenType() == DO_SYM)
        {
            lexer.GetWord();
            statement();
            // 无条件跳转至循环条件判断前
            pcodelist.emit(jmp, 0, condition);
        }
        else if (lexer.GetTokenType() & firstStatement)
        {
            errorHandle.error(MISSING, L"do", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            statement();
            // 无条件跳转至循环条件判断前
            pcodelist.emit(jmp, 0, condition);
        }
        else
            errorHandle.error(MISSING, L"do", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        pcodelist.backpatch(loop, pcodelist.code_list.size());
    }
    else if (lexer.GetTokenType() == CALL_SYM)
    { // <statement> -> call id ([{<exp>{,<exp>}])
        lexer.GetWord();
        ProcInfo *cur_info = nullptr;
        // <statement> -> call id
        if (lexer.GetTokenType() & IDENT)
        {
            int pos = symTable.SearchInfo(lexer.GetStrToken(), Category::PROCE);
            // 未查找到过程名
            if (pos == -1)
                errorHandle.error(UNDECLARED_PROC, lexer.GetStrToken().c_str(), lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            else
                cur_info = (ProcInfo *)symTable.table[pos].info;
            // 若调用未定义的过程
            if (cur_info && !cur_info->isDefined)
                errorHandle.error(UNDEFINED_PROC, lexer.GetStrToken().c_str(), lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            lexer.GetWord();
            if (lexer.GetTokenType() & LPAREN)
            {
                lexer.GetWord();
                if (lexer.GetTokenType() & firstExp)
                {
                    exp();
                    // 将实参传入即将调用的子过程
                    if (cur_info)
                        pcodelist.emit(store, -1, ACT_PRE_REC_SIZE + cur_info->level + 1 + 1);
                    size_t i = 1;
                    while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & firstExp))
                    {
                        if (lexer.GetTokenType() & COMMA)
                            lexer.GetWord();
                        else
                            errorHandle.error(MISSING, L",", lexer.GetPreWordRow(),
                                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                        if (lexer.GetTokenType() & firstExp)
                        {
                            exp();
                            // 将实参传入即将调用的子过程
                            if (cur_info)
                                pcodelist.emit(store, -1, ACT_PRE_REC_SIZE + cur_info->level + 1 + 1 + i++);
                        }
                        else
                            exp();
                    }
                    // 若实参列表与形参列表变量数不兼容，报错
                    if (cur_info && i != cur_info->formVarList.size())
                        errorHandle.error(INCOMPATIBLE_VAR_LIST, lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    if (lexer.GetTokenType() & RPAREN)
                    {
                        lexer.GetWord();
                        if (cur_info)
                            pcodelist.emit(call, cur_info->level, cur_info->entry);
                    }
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
                // 将实参传入即将调用的子过程
                if (cur_info)
                    pcodelist.emit(store, -1, ACT_PRE_REC_SIZE + cur_info->level + 2);
                size_t i = 1;
                while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & firstExp))
                {
                    if (lexer.GetTokenType() & COMMA)
                        lexer.GetWord();
                    else
                        errorHandle.error(MISSING, L",", lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    if (lexer.GetTokenType() & firstExp)
                    {
                        exp();
                        // 将实参传入即将调用的子过程
                        if (cur_info)
                            pcodelist.emit(store, -1, ACT_PRE_REC_SIZE + cur_info->level + 2 + i++);
                    }
                    else
                        exp();
                }
                // 若实参列表与形参列表变量数不兼容，报错
                if (cur_info && i != cur_info->formVarList.size())
                    errorHandle.error(INCOMPATIBLE_VAR_LIST, lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                if (lexer.GetTokenType() & RPAREN)
                {
                    lexer.GetWord();
                    if (cur_info)
                        pcodelist.emit(call, cur_info->level, cur_info->entry);
                }
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
                int pos = symTable.SearchInfo(lexer.GetStrToken(), Category::VAR);
                // 未查找到过程名
                VarInfo *cur_info = nullptr;
                if (pos == -1)
                    errorHandle.error(UNDECLARED_PROC, lexer.GetStrToken().c_str(), lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                else
                    cur_info = (VarInfo *)symTable.table[pos].info;
                if (cur_info)
                {
                    if (cur_info->cat == Category::CST)
                        errorHandle.error(ILLEGAL_RVALUE_ASSIGN, lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    // 从命令行读一个数据到栈顶
                    pcodelist.emit(red, 0, 0);
                    // 将栈顶值送入变量所在地址
                    pcodelist.emit(store, cur_info->level, cur_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + cur_info->level + 1);
                }
                lexer.GetWord();
                while (lexer.GetTokenType() & COMMA)
                {
                    lexer.GetWord();
                    if (lexer.GetTokenType() & IDENT)
                    {
                        int pos1 = symTable.SearchInfo(lexer.GetStrToken(), Category::VAR);
                        // 未查找到过程名
                        VarInfo *cur_info1 = nullptr;
                        if (pos1 == -1)
                            errorHandle.error(UNDECLARED_PROC, lexer.GetStrToken().c_str(), lexer.GetPreWordRow(),
                                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                        else
                            cur_info1 = (VarInfo *)symTable.table[pos1].info;
                        if (cur_info1)
                        {
                            if (cur_info1->cat == Category::CST)
                                errorHandle.error(ILLEGAL_RVALUE_ASSIGN, lexer.GetPreWordRow(),
                                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                            // 从命令行读一个数据到栈顶
                            pcodelist.emit(red, 0, 0);
                            // 将栈顶值送入变量所在地址
                            pcodelist.emit(store, cur_info1->level, cur_info1->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + cur_info1->level + 1);
                        }
                        lexer.GetWord();
                    }
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
                    {
                        int pos = symTable.SearchInfo(lexer.GetStrToken(), Category::VAR);
                        // 未查找到过程名
                        VarInfo *cur_info = nullptr;
                        if (pos == -1)
                            errorHandle.error(UNDECLARED_PROC, lexer.GetStrToken().c_str(), lexer.GetPreWordRow(),
                                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                        else
                            cur_info = (VarInfo *)symTable.table[pos].info;
                        if (cur_info)
                        {
                            if (cur_info->cat == Category::CST)
                                errorHandle.error(ILLEGAL_RVALUE_ASSIGN, lexer.GetPreWordRow(),
                                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                            // 从命令行读一个数据到栈顶
                            pcodelist.emit(red, 0, 0);
                            // 将栈顶值送入变量所在地址
                            pcodelist.emit(store, cur_info->level, cur_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + cur_info->level + 1);
                        }
                        lexer.GetWord();
                    }
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
                errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
        else if (lexer.GetTokenType() & IDENT)
        {
            errorHandle.error(MISSING, L"(", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            int pos = symTable.SearchInfo(lexer.GetStrToken(), Category::VAR);
            // 未查找到过程名
            VarInfo *cur_info = nullptr;
            if (pos == -1)
                errorHandle.error(UNDECLARED_PROC, lexer.GetStrToken().c_str(), lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            else
                cur_info = (VarInfo *)symTable.table[pos].info;
            if (cur_info)
            {
                if (cur_info->cat == Category::CST)
                    errorHandle.error(ILLEGAL_RVALUE_ASSIGN, lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                // 从命令行读一个数据到栈顶
                pcodelist.emit(red, 0, 0);
                // 将栈顶值送入变量所在地址
                pcodelist.emit(store, cur_info->level, cur_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + cur_info->level + 1);
            }
            lexer.GetWord();
            while (lexer.GetTokenType() & COMMA)
            {
                lexer.GetWord();
                if (lexer.GetTokenType() & IDENT)
                {
                    int pos1 = symTable.SearchInfo(lexer.GetStrToken(), Category::VAR);
                    // 未查找到过程名
                    VarInfo *cur_info1 = nullptr;
                    if (pos1 == -1)
                        errorHandle.error(UNDECLARED_PROC, lexer.GetStrToken().c_str(), lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    else
                        cur_info1 = (VarInfo *)symTable.table[pos1].info;
                    if (cur_info1)
                    {
                        if (cur_info1->cat == Category::CST)
                            errorHandle.error(ILLEGAL_RVALUE_ASSIGN, lexer.GetPreWordRow(),
                                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                        // 从命令行读一个数据到栈顶
                        pcodelist.emit(red, 0, 0);
                        // 将栈顶值送入变量所在地址
                        pcodelist.emit(store, cur_info1->level, cur_info1->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + cur_info1->level + 1);
                    }
                    lexer.GetWord();
                }
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
                {
                    int pos = symTable.SearchInfo(lexer.GetStrToken(), Category::VAR);
                    // 未查找到过程名
                    VarInfo *cur_info = nullptr;
                    if (pos == -1)
                        errorHandle.error(UNDECLARED_PROC, lexer.GetStrToken().c_str(), lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    else
                        cur_info = (VarInfo *)symTable.table[pos].info;
                    if (cur_info)
                    {
                        if (cur_info->cat == Category::CST)
                            errorHandle.error(ILLEGAL_RVALUE_ASSIGN, lexer.GetPreWordRow(),
                                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                        // 从命令行读一个数据到栈顶
                        pcodelist.emit(red, 0, 0);
                        // 将栈顶值送入变量所在地址
                        pcodelist.emit(store, cur_info->level, cur_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + cur_info->level + 1);
                    }
                    lexer.GetWord();
                }
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
                pcodelist.emit(wrt, 0, 0);
                while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & firstExp))
                {
                    if (lexer.GetTokenType() & COMMA)
                        lexer.GetWord();
                    else
                        errorHandle.error(MISSING, L",", lexer.GetPreWordRow(),
                                          lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    if (lexer.GetTokenType() & firstExp)
                    {
                        exp();
                        pcodelist.emit(wrt, 0, 0);
                    }
                    else
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
                    if (lexer.GetTokenType() & firstExp)
                    {
                        exp();
                        pcodelist.emit(wrt, 0, 0);
                    }
                    else
                        exp();
                }
                if (lexer.GetTokenType() & RPAREN)
                    lexer.GetWord();
                else if (lexer.GetTokenType() & followStatement)
                    errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            }
            else if (lexer.GetTokenType() & followStatement)
                errorHandle.error(MISSING, L")", lexer.GetPreWordRow(),
                                  lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        }
        else if (lexer.GetTokenType() & firstExp)
        {
            errorHandle.error(MISSING, L"(", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            exp();
            pcodelist.emit(wrt, 0, 0);
            while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & firstExp))
            {
                if (lexer.GetTokenType() & COMMA)
                    lexer.GetWord();
                else
                    errorHandle.error(MISSING, L",", lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                if (lexer.GetTokenType() & firstExp)
                {
                    exp();
                    pcodelist.emit(wrt, 0, 0);
                }
                else
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
                if (lexer.GetTokenType() & firstExp)
                {
                    exp();
                    pcodelist.emit(wrt, 0, 0);
                }
                else
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
        pcodelist.emit(opr, 0, 13); // 屏幕输出换行
    }
    else
        // system("pause");
        //  wcout << lexer.GetTokenType() << endl;
        judge(0, followStatement, ILLEGAL_DEFINE, L"statement");
}

//<exp> → [+|-]<term>{<aop><term>}
void Parser::exp()
{
    unsigned long aop = NUL;
    if (lexer.GetTokenType() & firstExp)
    {
        if (lexer.GetTokenType() & (PLUS | MINUS))
        {
            aop = lexer.GetTokenType();
            lexer.GetWord();
        }

        if (lexer.GetTokenType() & firstTerm)
        { //<exp> → [+|-]<term>
            term();
            if (aop & MINUS)
                pcodelist.emit(opr, 0, OPR_NEGTIVE);
            //<exp> → [+|-]<term>{<aop><term>}
            while (lexer.GetTokenType() & (PLUS | MINUS))
            {
                aop = lexer.GetTokenType();
                lexer.GetWord();
                if (lexer.GetTokenType() & firstTerm)
                {
                    term();
                    // 减
                    if (aop == MINUS)
                        pcodelist.emit(opr, 0, OPR_SUB);
                    // 加
                    else
                        pcodelist.emit(opr, 0, OPR_ADD);
                }
                else
                    errorHandle.error(REDUNDENT, lexer.GetStrToken().c_str(), lexer.GetPreWordRow(),
                                      lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
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
            unsigned long nop = lexer.GetTokenType();
            lexer.GetWord();
            if (lexer.GetTokenType() & firstFactor) //<term> → <factor>{<mop><factor>}
            {
                factor();
                // 乘
                if (nop == MULTI)
                    pcodelist.emit(opr, 0, OPR_MULTI);
                // 除
                else
                    pcodelist.emit(opr, 0, OPR_DIVIS);
            }
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
        // 非法的 term 开头
        judge(0, followTerm, ILLEGAL_DEFINE, L"term (invalid term start)");
}

//<factor>→<id>|<integer>|(<exp>)
void Parser::factor()
{
    if (lexer.GetTokenType() == IDENT)
    {
        // 查找变量符号
        int pos = symTable.SearchInfo(lexer.GetStrToken(), Category::VAR);
        VarInfo *cur_info = nullptr;
        if (pos == -1)
            errorHandle.error(UNDECLARED_IDENT, lexer.GetStrToken().c_str(),
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        else
            cur_info = (VarInfo *)symTable.table[pos].info;
        if (cur_info)
        {
            if (cur_info->cat == Category::CST)
            {
                int val = cur_info->GetValue();
                pcodelist.emit(lit, cur_info->level, val);
            }
            else
                pcodelist.emit(load, cur_info->level, cur_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + cur_info->level + 1);
        }
        lexer.GetWord();
    }
    else if (lexer.GetTokenType() == NUMBER)
    {
        pcodelist.emit(lit, 0, w_str2int(lexer.GetStrToken()));
        lexer.GetWord();
    }
    else if (lexer.GetTokenType() == LPAREN)
    {
        lexer.GetWord();
        exp();
        if (lexer.GetTokenType() == RPAREN)
            lexer.GetWord();
        else
            errorHandle.error(MISSING_DETAILED, L"')'",
                              L"Expected closing parenthesis ')'.",
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
    }
    else
        // 其他情况处理非法的 factor
        judge(0, followFactor, ILLEGAL_DEFINE, L"factor");
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
            unsigned int lop = lexer.GetTokenType();
            lexer.GetWord();
            exp();
            switch (lop)
            {
                // <
            case LSS:
                pcodelist.emit(opr, 0, OPR_LSS);
                break;
                // <=
            case LEQ:
                pcodelist.emit(opr, 0, OPR_LEQ);
                break;
                // >
            case GRT:
                pcodelist.emit(opr, 0, OPR_GRT);
                break;
                // >=
            case GEQ:
                pcodelist.emit(opr, 0, OPR_GEQ);
                break;
                // <>
            case NEQ:
                pcodelist.emit(opr, 0, OPR_NEQ);
                break;
                // =
            case EQL:
                pcodelist.emit(opr, 0, OPR_EQL);
                break;
            default:
                break;
            }
        }
        else
        {
            // 缺少逻辑操作符的情况
            errorHandle.error(MISSING,
                              L"Expected a logical operator (e.g., '=', '<>', '<') after the expression.",
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            lexer.GetWord();
            exp();
        }
    }
    else if (lexer.GetTokenType() & ODD_SYM)
    {
        lexer.GetWord();
        if (lexer.GetTokenType() & firstExp)
        {
            exp();
            pcodelist.emit(opr, 0, OPR_ODD);
        }
        else
            errorHandle.error(EXPECT, L"expression", lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
    }
    else
        // 非法的 lexp 开头
        judge(0, followLexp, ILLEGAL_DEFINE, L"lexp (invalid logical expression start)");
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
            symTable.InsertToTable(lexer.GetStrToken(), glo_offset, Category::VAR);
            glo_offset += 4;
            lexer.GetWord();
            while (lexer.GetTokenType() == COMMA)
            {
                lexer.GetWord();
                if (lexer.GetTokenType() & IDENT)
                {
                    symTable.InsertToTable(lexer.GetStrToken(), glo_offset, Category::VAR);
                    glo_offset += 4;
                    lexer.GetWord();
                }
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
                {
                    symTable.InsertToTable(lexer.GetStrToken(), glo_offset, Category::VAR);
                    glo_offset += 4;
                    lexer.GetWord();
                }
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
        symTable.InsertToTable(lexer.GetStrToken(), 0, CST);
        lexer.GetWord();
        //<const> → <id>:=<integer>
        if (lexer.GetTokenType() == ASSIGN)
            lexer.GetWord();
        else if (lexer.GetTokenType() != ASSIGN && lexer.GetTokenType() != NUMBER)
        {
            errorHandle.error(MISSING, L":=", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            lexer.GetWord();
        }
        else if (lexer.GetTokenType() == NUMBER)
        {
            errorHandle.error(MISSING, L":=", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            symTable.table[symTable.table.size() - 1].info->SetValue(lexer.GetStrToken());
            lexer.GetWord();
            return;
        }
        if (lexer.GetTokenType() == NUMBER)
        {
            symTable.table[symTable.table.size() - 1].info->SetValue(lexer.GetStrToken());
            lexer.GetWord();
        }
        else
            errorHandle.error(MISSING, L"[number]", lexer.GetPreWordRow(),
                              lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
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
    int flag = 0;
    if (lexer.GetTokenType() == PROC_SYM)
    {
        lexer.GetWord();
        ProcInfo *cur_info = nullptr; // 临时变量，记录当前过程符号表项的信息
        // <proc> -> procedure <id>
        if (lexer.GetTokenType() == IDENT)
        {
            symTable.MkTable();
            int cur_proc = symTable.InsertToTable(lexer.GetStrToken(), 0, Category::PROCE);
            if (cur_proc != -1)
            {
                cur_info = (ProcInfo *)symTable.table[cur_proc].info;
                // 子过程的入口地址登入符号表，待回填
                size_t entry = pcodelist.emit(jmp, 0, 0);
                symTable.table[symTable.table.size() - 1].info->SetEntry(entry);
            }
            lexer.GetWord();
            if (lexer.GetTokenType() & LPAREN)
            {
                // 层数增加
                symTable.display.push_back(0);
                symTable.level++;

                lexer.GetWord();
                if (lexer.GetTokenType() & IDENT)
                {
                    int form_var = symTable.InsertToTable(lexer.GetStrToken(), glo_offset, Category::FORM);
                    glo_offset += 4;
                    if (cur_info)
                        cur_info->formVarList.push_back(form_var);

                    lexer.GetWord();
                    while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & IDENT))
                    {
                        if (lexer.GetTokenType() & COMMA)
                            lexer.GetWord();
                        else
                            errorHandle.error(MISSING, L"','",
                                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                        if (lexer.GetTokenType() & IDENT)
                        {
                            int form_var = symTable.InsertToTable(lexer.GetStrToken(), glo_offset, Category::FORM);
                            glo_offset += 4;
                            if (cur_info)
                                cur_info->formVarList.push_back(form_var);
                            lexer.GetWord();
                        }
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
                        // 本层数据结束，记得回到上一层
                        pcodelist.emit(opr, 0, OPR_RETURN);
                        // 过程调用结束后,返回调用点并退栈
                        symTable.display.pop_back();
                        symTable.level--;

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
                        // 本层数据结束，记得回到上一层
                        pcodelist.emit(opr, 0, OPR_RETURN);
                        symTable.display.pop_back();
                        symTable.level--;
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
                symTable.display.push_back(0);
                symTable.level++;

                int form_var = symTable.InsertToTable(lexer.GetStrToken(), glo_offset, Category::FORM);
                glo_offset += 4;
                if (cur_info)
                    cur_info->formVarList.push_back(form_var);
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
                    {
                        int form_var = symTable.InsertToTable(lexer.GetStrToken(), glo_offset, Category::FORM);
                        glo_offset += 4;
                        if (cur_info)
                            cur_info->formVarList.push_back(form_var);
                        lexer.GetWord();
                    }
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
                        pcodelist.emit(opr, 0, OPR_RETURN);
                        // 本层数据结束，记得回到上一层
                        symTable.display.pop_back();
                        symTable.level--;

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
                        // 本层数据结束，记得回到上一层
                        pcodelist.emit(opr, 0, OPR_RETURN);
                        symTable.display.pop_back();
                        symTable.level--;

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
                // 层数增加
                symTable.display.push_back(0);
                symTable.level++;

                errorHandle.error(MISSING, L"'('",
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                lexer.GetWord();
                if (lexer.GetTokenType() & SEMICOLON)
                {
                    lexer.GetWord();
                    block();
                    // 本层数据结束，记得回到上一层
                    pcodelist.emit(opr, 0, OPR_RETURN);
                    symTable.level--;
                    symTable.display.pop_back();
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
                    // 本层数据结束，记得回到上一层
                    pcodelist.emit(opr, 0, OPR_RETURN);
                    symTable.level--;
                    symTable.display.pop_back();
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
            symTable.MkTable();
            int cur_proc = symTable.InsertToTable(L"null", 0, Category::PROCE);
            if (cur_proc != -1)
            {
                cur_info = (ProcInfo *)symTable.table[cur_proc].info;
                size_t entry = pcodelist.emit(jmp, 0, 0);
                symTable.table[symTable.table.size() - 1].info->SetEntry(entry);
            }
            // 层数增加
            symTable.display.push_back(0);
            symTable.level++;
            errorHandle.error(MISSING, L"'<id>'",
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());

            lexer.GetWord();
            if (lexer.GetTokenType() & IDENT)
            {
                int form_var = symTable.InsertToTable(lexer.GetStrToken(), glo_offset, Category::FORM);
                glo_offset += 4;
                if (cur_info)
                    cur_info->formVarList.push_back(form_var);
                lexer.GetWord();
                while ((lexer.GetTokenType() & COMMA) || (lexer.GetTokenType() & IDENT))
                {
                    if (lexer.GetTokenType() & COMMA)
                        lexer.GetWord();
                    else
                        errorHandle.error(MISSING, L"','",
                                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
                    if (lexer.GetTokenType() & IDENT)
                    {
                        int form_var = symTable.InsertToTable(lexer.GetStrToken(), glo_offset, Category::FORM);
                        glo_offset += 4;
                        if (cur_info)
                            cur_info->formVarList.push_back(form_var);
                        lexer.GetWord();
                    }
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
                    // 执行返回，并弹栈
                    pcodelist.emit(opr, 0, OPR_RETURN);
                    // 本层数据结束，记得回到上一层
                    symTable.level--;
                    symTable.display.pop_back();
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
                    // 本层数据结束，记得回到上一层
                    pcodelist.emit(opr, 0, OPR_RETURN);
                    symTable.level--;
                    symTable.display.pop_back();
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

        size_t cur_proc = symTable.sp;
        // wcout << L"now sp is: " << cur_proc << L"glo_offset: " << glo_offset << endl;
        ProcInfo *cur_info = (ProcInfo *)symTable.table[cur_proc].info;
        symTable.AddWidth(cur_proc, glo_offset);
        //<block> → [<condecl>][<vardecl>]<proc>
        if (lexer.GetTokenType() & firstProc)
            proc();
        // wcout << lexer.GetStrToken() << endl;
        //<block> → [<condecl>][<vardecl>][<proc>]<body>
        // 为子过程开辟活动记录空间，其中为display开辟level + 1个单元
        size_t entry = pcodelist.emit(alloc, 0, cur_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + symTable.level + 1);
        size_t target = cur_info->entry;
        // wcout<<cur_info->entry<<endl;
        // 将过程入口地址回填至过程的跳转语句
        pcodelist.backpatch(target, entry);
        // 过程体开始，过程已定义
        if (cur_proc)
            cur_info->isDefined = true;
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
        symTable.MkTable();
        symTable.EnterProgm(lexer.GetStrToken());
        lexer.GetWord();
        if (lexer.GetTokenType() == SEMICOLON)
        {
            lexer.GetWord();
            // 主过程的入口地址登入符号表，待回填
            size_t entry = pcodelist.emit(jmp, 0, 0);
            symTable.table[0].info->SetEntry(entry);
            block(); // 进入<block>
                     // 本层数据结束，记得回到上一层
            pcodelist.emit(opr, 0, OPR_RETURN);
            if (lexer.GetCh() != L'\0' && lexer.GetCh() != L'#')
                errorHandle.error(ILLEGAL_WORD, (L"'" + lexer.GetStrToken() + L"'").c_str(),
                                  lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            return;
        }
        else
        {
            // 主过程的入口地址登入符号表，待回填
            size_t entry = pcodelist.emit(jmp, 0, 0);
            symTable.table[0].info->SetEntry(entry);
            errorHandle.error(MISSING, L";",
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
            block();
            // 本层数据结束，记得回到上一层
            pcodelist.emit(opr, 0, OPR_RETURN);
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

        symTable.MkTable();
        symTable.EnterProgm(L"null");
        // TODO
        lexer.GetWord();
        // 主过程的入口地址登入符号表，待回填
        size_t entry = pcodelist.emit(jmp, 0, 0);
        symTable.table[0].info->SetEntry(entry);
        block(); // 进入<block>
                 // 本层数据结束，记得回到上一层
        pcodelist.emit(opr, 0, OPR_RETURN);
        if (lexer.GetCh() != '\0' && lexer.GetCh() != L'#')
            errorHandle.error(ILLEGAL_WORD, (L"'" + lexer.GetStrToken() + L"'").c_str(),
                              lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        return;
    }

    if (lexer.GetTokenType() & firstBlock)
    {
        symTable.MkTable();
        symTable.EnterProgm(L"null");
        // ERROR
        errorHandle.error(EXPECT_STH_FIND_ANTH, L"id", (L"'" + lexer.GetStrToken() + L"'").c_str(),
                          lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        // 主过程的入口地址登入符号表，待回填
        size_t entry = pcodelist.emit(jmp, 0, 0);
        symTable.table[0].info->SetEntry(entry);
        block(); // 进入<block>
        // 本层数据结束，记得回到上一层
        pcodelist.emit(opr, 0, OPR_RETURN);
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
