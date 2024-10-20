#include <SymTable.hpp>
SymTable symTable;

void Information::show()
{
    wcout << setw(10) << L"cat: " << setw(13) << cat_map[cat]
          << setw(10) << L"offset: " << setw(5) << offset
          << setw(10) << L"level: " << setw(5) << level;
}

void VarInfo::setValue(wstring val_str) { this->value = w_str2int(val_str); }

int VarInfo::getValue() { return this->value; }

void VarInfo::show()
{
    wcout << setw(10) << L"cat:" << setw(15) << cat_map[cat]
          << setw(10) << L"offset:" << setw(5) << offset
          << setw(10) << L"level:" << setw(5) << level
          << setw(10) << L"value:" << setw(5) << value;
}

void ProcInfo::show()
{

}

void SymTableItem::show()
{
    wcout << setw(10) << name ;
    info->show();
    wcout << endl;
}

void SymTable::clear()
{
    sp = 0;
    table.clear();
}