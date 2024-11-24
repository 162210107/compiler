#ifndef _TYPES_HPP
#define _TYPES_HPP
#include <locale>
#include <string>
#include <cstddef>
#include <fcntl.h>
#include <fstream>
#include <io.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <unordered_map>
#include <vector>
#include <windows.h>

#include <ostream>

// #include <wmcommn.h>
using namespace std;

// wchar_t 双字节字符类型,针对UNICODE编码格式
// wstring 双字节，针对UNICODE编码格式
const int RSV_WORD_MAX = 15; // 保留字的数量
const int OPR_MAX = 11;      // 操作数数量
const int ERR_CNT = 70;      // 报错种数

// 错误种类宏定义
#define EXPECT_STH_FIND_ANTH 0
#define EXPECT 1
#define EXPECT_NUMEBR_AFTER_BECOMES 2
#define ILLEGAL_DEFINE 3
#define ILLEGAL_WORD 4
#define ILLEGAL_RVALUE_ASSIGN 5
#define MISSING 6
#define REDUNDENT 7
#define UNDECLARED_IDENT 8
#define UNDECLARED_PROC 9
#define REDECLEARED_IDENT 10
#define REDECLEARED_PROC 11
#define INCOMPATIBLE_VAR_LIST 12
#define UNDEFINED_PROC 13
#define UNDEFINED_PROG 14
#define SYNTAX_ERROR 15
#define MISSING_DETAILED 16
#define INVALID_SYNTAX 17
#define UNEXPECTED_TOKEN 18

#define NUL 0x0           /* 空 */

//<lop>
#define EQL 0x1           /* =  1*/
#define NEQ 0x2           /* <> 2*/
#define LSS 0x4           /* < 4*/
#define LEQ 0x8           /* <= 8*/
#define GRT 0x10          /* > 16*/
#define GEQ 0x20          /* >= 32*/

//<aop>
#define PLUS 0x40        /* + */
#define MINUS 0x80       /* - */

//<mop>
#define MULTI 0x100       /* * */
#define DIVIS 0x200      /* / */

//<id>
#define IDENT 0x400        /* 标识符 */

//<integer>
#define NUMBER 0x800      /* 数值 */

#define LPAREN 0x1000     /* ( */
#define RPAREN 0x2000     /* ) */
#define COMMA 0x4000      /* , */
#define SEMICOLON 0x8000 /* ; */
#define ASSIGN 0x10000    /*:=*/

//保留字
#define ODD_SYM 0x20000      /* 奇数判断 64*/
#define BEGIN_SYM 0x40000
#define END_SYM 0x80000
#define IF_SYM 0x100000
#define THEN_SYM 0x200000
#define WHILE_SYM 0x400000
#define DO_SYM 0x800000
#define CALL_SYM 0x1000000
#define CONST_SYM 0x2000000
#define VAR_SYM 0x4000000
#define PROC_SYM 0x8000000
#define WRITE_SYM 0x10000000
#define READ_SYM 0x20000000
#define PROGM_SYM 0x40000000
#define ELSE_SYM 0x80000000

#ifndef UNICODE
#define UNICODE
#endif
class ReadUnicode
{
private:
    ifstream file;
    wstring progm_w_str; // 源程序代码的wchar字符串形式

public:
    void InitReadUnicode();
    void readFile2USC2(string);
    wchar_t getProgmWStr(const size_t nowPtr);
    bool isEmpty();
};

int w_str2int(wstring num_str);

extern ReadUnicode readUnicode;

#endif