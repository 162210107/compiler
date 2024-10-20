#ifndef _ERROR_HANDLE_HPP
#define _ERROR_HANDLE_HPP
#include <lexer.hpp>
#include <types.hpp>

class ErrorHandle
{
private:
    unsigned int err_cnt;     // 出错总次数
    wstring err_msg[ERR_CNT]; // 错误信息表

    void printPreWord(const wchar_t msg[],const size_t preWordRow,const size_t preWordCol);
    void printCurWord(const wchar_t msg[],const size_t rowPos,const size_t colPos);
    void over();
    // template <class... T>
    // void error(unsigned int n, T... extra,const size_t preWordRow,const size_t preWordCol,const size_t rowPos,const size_t colPos)
    // {
    //     wchar_t msg[200] = L"";
    //     wsprintfW(msg, err_msg[n].c_str(), extra...);
    //     err_cnt++;
    //     if (n == REDUNDENT || n == MISSING || n == UNDECLARED_PROC)
    //         printPreWord(msg,preWordRow,preWordCol);
    //     else
    //         printCurWord(msg,rowPos,colPos);
    // }
public:
    void error(const unsigned int n,const size_t preWordRow,const size_t preWordCol,const size_t rowPos,const size_t colPos);
    void error(const unsigned int n, const wchar_t *,const size_t preWordRow,const size_t preWordCol,const size_t rowPos,const size_t colPos);
    void error(const unsigned int n, const wchar_t *, const wchar_t *,const size_t preWordRow,const size_t preWordCol,const size_t rowPos,const size_t colPos);
    void InitErrorHandle();
};

extern ErrorHandle errorHandle;

#endif