#ifndef _P_CODE_H
#define _P_CODE_H
#include <PCode.hpp>
#include <types.hpp>

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

PCodeList pcodelist;
#endif