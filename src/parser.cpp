#include <parser.hpp>
Parser parser;

// 用于错误恢复的函数，若当前符号在s1中，则读取下一符号；若当前符号不在s1中，则报错，接着循环查找下一个在中s1 ∪ s2的符号
void Parser::judge(const unsigned long s1, const unsigned long s2, const unsigned int n,const wchar_t* extra)
{
    if (!(lexer.GetTokenType() & s1)) // 当前符号不在s1中
    {
        errorHandle.error(n, extra,lexer.GetPreWordRow(),
            lexer.GetPreWordCol(),lexer.GetRowPos(),lexer.GetColPos());
        unsigned long s3 = s1 | s2; // 把s2补充进s1

        while (!(lexer.GetTokenType() & s3)) // 循环找到下一个合法的符号
        {
            if (lexer.GetCh() == L'\0')
                errorHandle.over();
            lexer.GetWord(); // 继续词法分析
        }
        if (lexer.GetTokenType() & s1)
            lexer.GetWord();
    } else
        lexer.GetWord();
}

void Parser::judge(const unsigned long s1, const unsigned long s2, const unsigned int n,const wchar_t* extra1,const wchar_t* extra2)
{
    if (!(lexer.GetTokenType() & s1)) // 当前符号不在s1中
    {
        errorHandle.error(n, extra1,extra2,lexer.GetPreWordRow(),
            lexer.GetPreWordCol(),lexer.GetRowPos(),lexer.GetColPos());
        unsigned long s3 = s1 | s2; // 把s2补充进s1

        while (!(lexer.GetTokenType() & s3)) // 循环找到下一个合法的符号
        {
            if (lexer.GetCh() == L'\0')
                errorHandle.over();
            lexer.GetWord(); // 继续词法分析
        }
        if (lexer.GetTokenType() & s1)
            lexer.GetWord();
    } else
        lexer.GetWord();
}

/*<lop> → =|<>|<|<=|>|>=
<aop> → +|-
<mop> → *|/
<id> → l{l|d}   （注：l表示字母）
<integer> → d{d}*/

//<block> → [<condecl>][<vardecl>][<proc>]<body>
void Parser::block()
{
}

//<proc> → procedure <id>（[<id>{,<id>}]）;<block>{;<proc>}
void Parser::proc()
{
}

/*<statement> → <id> := <exp>
               |if <lexp> then <statement>[else <statement>]
               |while <lexp> do <statement>
               |call <id>（[<exp>{,<exp>}]）
               |<body>
               |read (<id>{，<id>})
               |write (<exp>{,<exp>})*/
void Parser::statement()
{
}

//<condecl> → const <const>{,<const>};
void Parser::condecl()
{
}

//<const> → <id>:=<integer>
void Parser::constA()
{
}

//<exp> → [+|-]<term>{<aop><term>}
void Parser::expression()
{
}

//<term> → <factor>{<mop><factor>}
void Parser::term()
{
}

//<vardecl> → var <id>{,<id>};
void Parser::vardecl()
{
}

//<factor>→<id>|<integer>|(<exp>)
void Parser::factor()
{
}

//<body> → begin <statement>{;<statement>}end
void Parser::body()
{
}

//<lexp> → <exp> <lop> <exp>|odd <exp>
void Parser::lexp()
{
}

//<prog> → program <id>；<block>
void Parser::prog()
{
    judge(PROGM_SYM,0,MISSING,L"progrsm");

    //<prog> → program <id>
    if(lexer.GetTokenType()==IDENT){
        symTable.InsertToTable(lexer.GetStrToken(),Category::PROCE);
        symTable.MkTable();
    }//<prog> → program ;
    else if(lexer.GetTokenType()==SEMICOLON){
        //ERROR
        errorHandle.error(MISSING,L"program name",
            lexer.GetPreWordRow(),lexer.GetPreWordCol(),lexer.GetRowPos(),lexer.GetColPos());
        
        //TODO
        
        lexer.GetWord();
        block();//进入<block>
        return;
    }//<prog> → program illegal_word
    else{ILLEGAL_WORD,(L"'"+lexer.GetStrToken()+L"'"),
        //ERROR
        errorHandle.error(EXPECT_STH_FIND_ANTH,L"id",(L"'"+lexer.GetStrToken()+L"'").c_str(),
            lexer.GetPreWordRow(),lexer.GetPreWordCol(),lexer.GetRowPos(),lexer.GetColPos());
    }

    lexer.GetWord();
    //<prog> → program <id> ; <block>
    judge(SEMICOLON,firstBlock,MISSING,L";");

    //TODO

    
    block();
}