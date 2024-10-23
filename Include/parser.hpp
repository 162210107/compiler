#ifndef PARSER_HPP
#define PARSER_HPP

#include <types.hpp>
#include <ErrorHandle.hpp>
#include <SymTable.hpp>
#include <lexer.hpp>

// block、proc、statement、condition、expression、term、factor

class Parser
{
private:
    // first集
    unsigned long firstProg = PROGM_SYM;
    unsigned long firstBlock = firstCondecl | firstVardecl | firstProc | firstBody;
    unsigned long firstCondecl = CONST_SYM;
    unsigned long firstConst = IDENT;
    unsigned long firstVardecl = VAR_SYM;
    unsigned long firstProc = PROC_SYM;
    unsigned long firstBody = BEGIN_SYM;
    unsigned long firstStatement = IDENT | IF_SYM | WHILE_SYM | CALL_SYM | firstBody | READ_SYM | WRITE_SYM;
    unsigned long firstLexp = firstExp | ODD_SYM;
    unsigned long firstExp = firstTerm | PLUS | MINUS;
    unsigned long firstTerm = firstFactor;
    unsigned long firstFactor = IDENT | NUMBER | LPAREN;
    unsigned long firstLop = EQL | NEQ | LSS | LEQ | GRT | GEQ;

    // follow集
    unsigned long followProg = 0;
    unsigned long followBlock = SEMICOLON | NUL;
    unsigned long followCondecl = firstVardecl | firstProc | firstBody;
    unsigned long followConst = COMMA | SEMICOLON;
    unsigned long followVardecl = firstProc | firstBody;
    unsigned long followProc = firstBody | SEMICOLON; 
    unsigned long followBody = SEMICOLON | followStatement;
    unsigned long followStatement = SEMICOLON | END_SYM | ELSE_SYM;
    unsigned long followLexp = THEN_SYM | DO_SYM;
    unsigned long followExp = firstLop | COMMA | RPAREN | followStatement | followLexp; 
    unsigned long followTerm = followExp | PLUS | MINUS;
    unsigned long followFactor = followTerm | MULTI | DIVIS;
    unsigned long followLop = followExp | followFactor;
    unsigned long followId = COMMA | SEMICOLON | LPAREN | RPAREN | followFactor;
public:
    void block();
    void proc();
    void statement();
    void constA();
    void condecl();
    void vardecl();
    void expression();
    void term();
    void factor();
    void prog();
    void body();
    void lexp();
    void Parser::judge(const unsigned long s1, const unsigned long s2, const unsigned int n,const wchar_t* extra);
    void Parser::judge(const unsigned long s1, const unsigned long s2, const unsigned int n,const wchar_t* extra1,const wchar_t* extra2);
};

extern Parser parser;

#endif