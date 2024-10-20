#include <ErrorHandle.hpp>

void ErrorHandle::InitErrorHandle()
{
    err_cnt = 0;
    // 报错信息初始化
    // missing错误
    err_msg[MISSING] = L"Missing %s";
    // undeclare错误
    err_msg[UNDECLARED_IDENT] = L"Undeclared identifier '%s'";
    err_msg[UNDECLARED_PROC] = L"Undeclared procedure name '%s'";
    // redefined错误
    err_msg[REDECLEARED_IDENT] = L"Redecleared identifier '%s'";
    err_msg[REDECLEARED_PROC] = L"Redecleared procedure name '%s'";
    // illegal错误
    err_msg[ILLEGAL_DEFINE] = L"Illegal %s definition ";
    err_msg[ILLEGAL_WORD] = L"Illegal word %s";
    err_msg[ILLEGAL_RVALUE_ASSIGN] = L"Cannot assign a rvalue";
    // expect错误
    err_msg[EXPECT] = L"Expecting %s";
    err_msg[EXPECT_STH_FIND_ANTH] = L"Expecting %s but %s was found";
    // redundant错误
    err_msg[REDUNDENT] = L"Redundent %s";
    // 其他错误
    err_msg[INCOMPATIBLE_VAR_LIST] = L"The real variable list is incompatible with formal variable list";
    err_msg[UNDEFINED_PROC] = L"Calling undefined procedure '%s'";
}

// 打印错误信息
void ErrorHandle::printPreWord(const wchar_t msg[],const size_t preWordRow,const size_t preWordCol)
{
    wcout << L"(" << preWordRow << "," << preWordCol << L")"
          << L" Error: " << msg << endl;
}

void ErrorHandle::printCurWord(const wchar_t msg[],const size_t rowPos,const size_t colPos)
{
    wcout << L"(" << rowPos << "," << colPos << L")"
          << L" Error: " << msg << endl;
}

void ErrorHandle::error(const unsigned int n,const size_t preWordRow,const size_t preWordCol,const size_t rowPos,const size_t colPos)
{
    wchar_t msg[200] = L"";
    // wsprintfW(msg, err_msg[n].c_str());
    swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), err_msg[n].c_str());
    err_cnt++;
    if (n == REDUNDENT || n == MISSING || n == UNDECLARED_PROC)
        printPreWord(msg,preWordRow,preWordCol);
    else
        printCurWord(msg,rowPos,colPos);
}

void ErrorHandle::error(const unsigned int n, const wchar_t* extra,const size_t preWordRow,const size_t preWordCol,const size_t rowPos,const size_t colPos)
{
    wchar_t msg[200] = L"";
    // wsprintfW(msg, err_msg[n].c_str(), extra);
    swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), err_msg[n].c_str(), extra);
    err_cnt++;
    if (n == REDUNDENT || n == MISSING || n == UNDECLARED_PROC)
        printPreWord(msg,preWordRow,preWordCol);
    else
        printCurWord(msg,rowPos,colPos);
}

void ErrorHandle::error(const unsigned int n, const wchar_t* extra1, const wchar_t* extra2,const size_t preWordRow,const size_t preWordCol,const size_t rowPos,const size_t colPos)
{
    wchar_t msg[200] = L"";
    // wsprintfW(msg, err_msg[n].c_str(), extra1, extra2);
    swprintf_s(msg, sizeof(msg) / sizeof(wchar_t), err_msg[n].c_str(), extra1, extra2);
    err_cnt++;
    if (n == REDUNDENT || n == MISSING || n == UNDECLARED_PROC)
        printPreWord(msg,preWordRow,preWordCol);
    else
        printCurWord(msg,rowPos,colPos);
}
// 格式化输出错误分析结果
void ErrorHandle::over()
{
    if (err_cnt == 0) {
        wcout << L"No error. Congratulations!" << endl;
        wcout << L"______________________________Compile compelete!________________________________\n"
              << endl;
    } else {
        wcout << L"Totol: " << err_cnt << L" errors" << endl;
        wcout << L"_______________________________Compile failed!_________________________________\n"
              << endl;
    }
}

ErrorHandle errorHandle;