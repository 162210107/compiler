#include <PCode.hpp>
PCodeList pcodelist;

int PCodeList::emit(Operation op, int L, int a)
{
    code_list.push_back(PCode(op, L, a));
    return code_list.size() - 1;
}

void PCodeList::backpatch(size_t target, size_t addr)
{
    if (addr == -1)
        return;
    else
        code_list[target].a = addr;
}

void PCodeList::printCode()
{
    for (size_t i = 0; i < code_list.size(); i++) 
        wcout << setw(4) << i << L"  " << op_map[code_list[i].op] << L", " << code_list[i].L << L", " << code_list[i].a << endl;
}