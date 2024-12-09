#include <SymTable.hpp>
SymTable symTable;

void Information::show()
{
    wcout << setw(10) << L"cat: " << setw(13) << cat
          << setw(10) << L"offset: " << setw(5) << offset
          << setw(10) << L"level: " << setw(5) << level;
}

void VarInfo::SetValue(wstring val) { this->value = w_str2int(val); }

int VarInfo::GetValue() { return this->value; }

void VarInfo::show()
{
    wcout << setw(10) << L"cat:" << setw(15) << cat
          << setw(10) << L"offset:" << setw(5) << offset
          << setw(10) << L"level:" << setw(5) << level
          << setw(10) << L"value:" << setw(5) << value;
}

SymTableItem SymTable::GetTable(int num)
{
    return table.at(num);
}

ProcInfo::ProcInfo()
    : Information()
{
    this->entry = 0;
    this->isDefined = false;
}

void ProcInfo::show()
{
    wcout << setw(10) << L"cat:" << setw(15) << cat
          << setw(10) << L"size:" << setw(5) << offset
          << setw(10) << L"level:" << setw(5) << level
          << setw(10) << L"entry:" << setw(5) << entry
          << setw(17) << L"form var list:";
    if (formVarList.empty())
        wcout << setw(5) << L"null";
    for (size_t mem : formVarList)
        wcout << setw(5) << symTable.GetTable(mem).name;
}

// 输入：词法分析器识别到的词法name，cat（类别）
// 功能：找到距离当前层最近出现的相同的词，并返回该词在符号表中的位置
// 输出：-1表示找不到，不是-1表示找得到
int SymTable::SearchInfo(wstring name, Category cat)
{
    unsigned int curAddr = 0;
    if (level == 0 && display[0] == 0) // 说明第0层啥也没有，肯定没有重复的
        return -1;
    for (int i = level; i >= 0; i--)
    {                         // 遍历所有子过程
        curAddr = display[i]; // 当前子过程其实符号表项
        while (1)
        {
            if (cat == Category::PROCE)
            {
                if (table[curAddr].info->cat == Category::PROCE && table[curAddr].name == name)
                    return curAddr;
                if (table[curAddr].previous == 0) // previous为0时说明这是本层的第一个，到头了
                    break;
                curAddr = table[curAddr].previous;
            }
            else
            {
                // 有可能其他也是这个名字，所以要匹配所有
                if (table[curAddr].info->cat != Category::PROCE && table[curAddr].name == name)
                    return curAddr;
                if (table[curAddr].previous == 0) // previous为0时说明这是本层的第一个，到头了
                    break;
                curAddr = table[curAddr].previous;
            }
        }
    }
    return -1;
}

// 功能：每当进入一个新的子程序时（已经确定是，但子程序名还没有写入）
// 就更新sp到子程序在符号表中的起始位置
void SymTable::MkTable()
{
    sp = table.size();
}

void SymTableItem::show()
{
    wcout << setw(10) << name << setw(10) << previous;
    info->show();
    wcout << endl;
}

// 输入：词法分析器识别到的词法name，cat（类别）
// 功能：函数归为当前层，遇到”（“才进入下一层，然后level++，继续存消息
// 输出：成功返回插入符号表的位置，失败返回-1
int SymTable::InsertToTable(wstring name, size_t offset, Category cat)
{
    int pos = SearchInfo(name, cat);
    if (cat == Category::PROCE && pos != -1 && table[pos].info->level == level + 1) // 找到的相同的词在本层
    {
        errorHandle.error(REDECLEARED_PROC, name.c_str(), lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        return -1;
    }
    else if (pos != -1 && table[pos].info->level == level && cat != Category::PROCE) // 如果查找到重复符号，且必须在同一层级，不为形参、过程名，则说明出现变量名重定义
    {
        errorHandle.error(REDECLEARED_IDENT, name.c_str(), lexer.GetPreWordRow(), lexer.GetPreWordCol(), lexer.GetRowPos(), lexer.GetColPos());
        return -1;
    }

    size_t curAddr = table.size();
    SymTableItem item;
    item.name = name;
    // 更新本层最新的符号表项
    item.previous = display[level];
    display[level] = curAddr;

    if (cat == Category::PROCE)
    {
        ProcInfo *procInfo = new ProcInfo;
        procInfo->cat = cat;
        procInfo->entry = 0;
        procInfo->offset = 0;
        procInfo->level = level + 1;
        item.info = procInfo;
    }
    else
    {
        VarInfo *varInfo = new VarInfo;
        varInfo->offset = offset;
        varInfo->cat = cat;
        varInfo->level = level;
        varInfo->SetValue(0);
        item.info = varInfo;
    }
    table.push_back(item);
    return curAddr;
}

void SymTable::EnterProgm(wstring name)
{
    SymTableItem item;
    item.previous = 0;
    item.name = name;
    ProcInfo* procInfo = new ProcInfo;
    procInfo->offset = 0;
    procInfo->cat = Category::PROCE;
    procInfo->level = 0;
    item.info = procInfo;
    table.push_back(item);
}

void SymTable::showAll()
{
    wcout << L"____________________________________________________SymTable_______________________________________________" << endl;
    for (SymTableItem mem : SymTable::table) {
        mem.show();
    }
    wcout << L"___________________________________________________________________________________________________________" << endl;
}

void SymTable::AddWidth(size_t addr, size_t width)
{
    table[addr].info->offset = width;
    glo_offset = 0;
}

void SymTable::InitAndClear()
{
    sp = 0;
    table.clear();
    display.clear();
    table.reserve(100);
    display.resize(1, 0);
}