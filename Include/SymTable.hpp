#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H
#include <types.hpp>
#include <ErrorHandle.hpp>
using namespace std;

enum Type
{
    INTERGER
};

enum Category
{
    NIL,   // 空
    ARR,   // 数组
    VAR,   // 变量
    PROCE, // 过程
    CST,   // 常量
    FORM,  // 形参
};

// 父类信息类型
class Information
{
public:
    wstring cat_map[6] = {
        L"null",
        L"array",
        L"var",
        L"procedure",
        L"const",
        L"formal var"};

    enum Category cat; // 种属
    size_t offset;
    size_t level;

    Information():offset(0),cat(Category::NIL),level(0){};
    virtual void setValue(wstring val_str) {}
    virtual int getValue() { return 0; }
    virtual void setEntry(size_t entry) {}
    virtual size_t getEntry() { return -1; }
    virtual void show();
};

// 变量信息，继承信息类型
class VarInfo : public Information
{
private:
    enum Type type; // 类型
    int value;      // 值

public:
    VarInfo() : Information(), type(Type::INTERGER), value(0) {};
    void setValue(wstring val_str) override;
    int getValue() override;
    void show() override;
};

// 过程信息，继承信息类型
class ProcInfo : public Information
{
private:

public:
    ProcInfo() : Information(){};
    void show() override;
};

// 符号表项
class SymTableItem
{
public:
    Information *info;  //不同类型对应不同的info
    wstring name;       // 符号名
    void show();
};

// 符号表
class SymTable
{
private:
    size_t sp;                  // 指向当前子过程符号表的首地址
    vector<SymTableItem> table; // 一个程序唯一的符号表

public:
    SymTable() : sp(0) {};
   
    // 清空符号表
    void clear();
};

extern SymTable symTable;

#endif