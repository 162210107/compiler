#include <types.hpp>
#include <lexer.hpp>
#include <ErrorHandle.hpp>
using namespace std;

void init()
{
    readUnicode.InitReadUnicode();
    // 清空原来的文件字符串
    lexer.InitLexer();
    // InitLexer();
    errorHandle.InitErrorHandle();
    // 以Unicode方式打开输入输出流
    _setmode(_fileno(stdout), _O_U16TEXT);
}

void TestLexer()
{
    string filename = "";
    while (cin >> filename)
    {
        init();
        //C:\Users\蔡蕾\Desktop\compilation-principle\test\test_error.txt
        readUnicode.readFile2USC2("D:\\compilation-principle\\test\\" + filename);
        if (readUnicode.isEmpty())
        {
            wcout << L"请输入下一个待编译的文件名称, 或输入'r'重复, 或按Ctrl+C结束" << endl;
            continue;
        }

        lexer.GetWord();
        while(lexer.GetCh()!=L'\0'){
            lexer.GetWord();
        }
        wcout<<L"finished!";
        return;
    }
}

int main()
{
    TestLexer();
}
