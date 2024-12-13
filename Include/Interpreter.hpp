#ifndef _INTER_H
#define _INTER_H
#include <PCode.hpp>
#include <types.hpp>
using namespace std;

#define RA 0
#define DL 1
#define GLO_DIS 2 // 全局display存放位置
#define DISPLAY 3 // 具体的display存放位置
// P代码的解释器
class Interpreter {
public:
    size_t pc;                 // 指令寄存器
    size_t top;                // 活动记录栈顶，并非实际开辟的空间栈顶
    size_t sp;                 // 当前活动记录基地址
    vector<int> running_stack; // 数据运行栈    
    void run();
    
private:
    void lit(Operation op, int L, int a);
    void opr(Operation op, int L, int a);
    void lod(Operation op, int L, int a);
    void sto(Operation op, int L, int a);
    void cal(Operation op, int L, int a);
    void alc(Operation op, int L, int a);
    void jmp(Operation op, int L, int a);
    void jpc(Operation op, int L, int a);
    void red(Operation op, int L, int a);
    void wrt(Operation op, int L, int a);
    void clear();
    void Init();
};

extern Interpreter interpreter;
#endif