#ifndef _SYMBOL_TABLE_HPP
#define _SYMBOL_TABLE_HPP
#include <types.hpp>
#include <lexer.hpp>
using namespace std;

//符号表元素类型
enum Category
{
    NIL,   // 空
    VAR,   // 变量
    PROCE, // 过程
    CST,   // 常量
    FORM,  // 形参
    PROG,  // 程序刚开始
};

// 父类信息类
class Information
{
public:
    Category cat; // 种属
    size_t level; // 属于的子程序层数
    size_t offset;
    size_t entry;

    Information():cat(Category::NIL),level(0),offset(0),entry(-1){};
    virtual void SetValue(wstring value) {}
    virtual int GetValue() { return -1; }
    virtual void show();
    virtual void SetEntry(size_t entry) { };
    virtual size_t GetEntry(){ return -1;};
};

// 变量信息，继承信息类型
class VarInfo : public Information
{
private:
    int value;      // 变量值
public:
    VarInfo() : Information(),value(0) {};
    void SetValue(wstring val) override;
    void SetValue(int nowValue){value=nowValue;};
    int GetValue() override;

    void show() override;
};

// 过程信息，继承信息类型
class ProcInfo : public Information
{
public:
    bool isDefined;             // 过程是否定义的标识
    vector<size_t> formVarList; // 过程的形参入口地址列表

    ProcInfo();
    void show() override;
    void SetEntry(size_t entry) override;
    size_t GetEntry()override;
};

// 符号表项
class SymTableItem
{
public:
    Information *info;  // 不同类型对应不同的info
    wstring name;       // 符号名
    size_t previous;    // 指针域，链接它在同一过程内的前以域名字在表中的下标
    
    void show();
};

// 符号表
class SymTable
{
public:
    size_t sp;                  // 指向当前子过程名字的地址,符号表从0开始
    vector<SymTableItem> table; // 一个程序唯一的符号表
    vector<size_t> display;     // 过程的嵌套层次表，栈结构进入新的一层起始处的符号表（其实是链表末尾下标）
    size_t level;               // 记录当前程序层级

public:
    SymTable() : sp(0) ,level(0){display.resize(1, 0);};
    SymTableItem GetTable(int num);
    void PopDisplay(){display.pop_back();}

    void EnterProgm(wstring name);
    void showAll();
    int InsertToTable(wstring name, size_t offset,Category cat);  // 插入表格
    int SearchInfo(wstring name,Category cat);      // 查找过程名在符号表中位置，主过程返回-1
    void MkTable();                                 // 进入新的过程，获取新的过程起始位置sp
    void InitAndClear();                                   // 清空整个程序的符号表
    void AddWidth(size_t addr, size_t width);
};

extern SymTable symTable;

#endif