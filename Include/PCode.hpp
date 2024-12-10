#ifndef _P_CODE_H
#define _P_CODE_H
#include <types.hpp>

// 中间代码指令集
enum Operation {
    lit,       // 取常量a放入数据栈栈顶
    opr,       // 执行运算，a表示执行某种运算
    load,      // 取变量（相对地址为a，层差为L）放到数据栈的栈顶
    store,     // 将数据栈栈顶的内容存入变量（相对地址为a，层次差为L）
    call,      // 调用过程（转子指令）（入口地址为a，层次差为L）
    alloc,     // 数据栈栈顶指针增加a
    jmp,       // 条件转移到地址为a的指令
    jpc,       // 条件转移指令，转移到地址为a的指令
    red,       // 读数据并存入变量（相对地址为a，层次差为L）
    wrt,       // 将栈顶内容输出
};

wstring op_map[P_CODE_CNT] = {
    L"LIT",
    L"OPR",
    L"LOD",
    L"STO",
    L"CAL",
    L"INT",
    L"JMP",
    L"JPC",
    L"RED",
    L"WRT"
};

class PCode {
public:
    Operation op; // 伪操作码
    int L;        // 层级
    int a;        // 相对地址

    PCode(Operation op1, int L1, int a1): op(op1),L(L1),a(a1){};
};

class PCodeList {
public:
    vector<PCode> code_list;

    int emit(Operation op, int L, int a);
    void backpatch(size_t target, size_t addr);
    void printCode();
    void clear(){code_list.clear();};
};

extern PCodeList pcodelist;
#endif