#ifndef _LEXER_HPP
#define _LEXER_HPP
#include <types.hpp>
#include <ErrorHandle.hpp>
using namespace std;

class Lexer
{
private:
    wchar_t ch;                                    // 存放最新读进的字符
    unsigned long tokenType;                       // 最近一次识别出来的 token 的类型
    wstring strToken;                              // 存放构成单词符号的字符串
    size_t nowPtr;                                 // 字符指针，指向词法分析当前读取的字符
    size_t colPos;                                 // 列指针，搜索指示器
    size_t rowPos;                                 // 行指针，搜索指示器
    size_t preWordCol;                             // 上一个非空白合法词尾列指针
    size_t preWordRow;                             // 上一个非空白合法词行指针
    unordered_map<unsigned long, wstring> sym_map; // 保留字编号与字符串的映射
    // 保留字表
    wstring resv_table[RSV_WORD_MAX] = {
        L"odd", L"begin", L"end", L"if", L"then", L"while", L"do", L"call",
        L"const", L"var", L"procedure", L"write", L"read", L"program", L"else"};
    // 运算符号表
    wchar_t opr_table[OPR_MAX] = {L'+', L'-', L'*', L'/', L'=', L'<',
                                  L'>', L'(', L')', L',', L';'};

    bool IsDigit();    // 是否是数字
    bool IsLetter();   // 是否是字母
    bool IsBoundary(); // 是否是界符
    int IsOperator();  // 是否是操作符
    void GetBC();      // 使当前指针跳过连续空格到下一个非空格处
    void GetChar();    // 读取下一个字符
    void Retract();    // 回退超前搜索的字符
    int Reserve();     // 判断读到的字符是否是关键字
    void Concat();     // 追加字符串到结果表

public:
    void GetWord();
    void InitLexer();
    wchar_t GetCh();
    size_t GetPreWordCol(){return preWordCol;};
    size_t GetPreWordRow(){return preWordRow;};
    size_t GetColPos(){return colPos;};
    size_t GetRowPos(){return rowPos;};
};

extern Lexer lexer;

#endif