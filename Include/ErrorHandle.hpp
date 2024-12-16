#ifndef _ERROR_HANDLE_HPP
#define _ERROR_HANDLE_HPP
#include <lexer.hpp>
#include <types.hpp>

class ErrorHandle
{
private:
    unsigned int errCnt;     // 出错总次数
    wstring errMsg[ERR_CNT]; // 错误信息表

    void printPreWord(const wchar_t msg[],const size_t preWordRow,const size_t preWordCol);
    void printCurWord(const wchar_t msg[],const size_t rowPos,const size_t colPos);
    
public:
    void error(const unsigned int n,const size_t preWordRow,const size_t preWordCol,const size_t rowPos,const size_t colPos);
    void error(const unsigned int n, const wchar_t *,const size_t preWordRow,const size_t preWordCol,const size_t rowPos,const size_t colPos);
    void error(const unsigned int n, const wchar_t *, const wchar_t *,const size_t preWordRow,const size_t preWordCol,const size_t rowPos,const size_t colPos);
    void InitErrorHandle();
    unsigned int GetError(){return errCnt;};
    void over();
};

extern ErrorHandle errorHandle;

#endif