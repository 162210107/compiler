#include <types.hpp>

using namespace std;

// wstring 转 int
int w_str2int(wstring num_str)
{
    if (num_str.empty()) {
        wcout << L"Cannot transfer empty string!" << endl;
        return 0;
    }
    int num = 0;
    // 先遍历一遍字符串，判断合法性
    size_t size = num_str.size();
    for (wchar_t w_ch : num_str) {
        if (!(w_ch <= L'9' && w_ch >= L'0')) {
            wcout << L"Illegal string to transfer!" << endl;
            return 0;
        }
    }
    for (size_t i = 0; i < size; i++) {
        num = (num << 3) + (num << 1); // num*10
        num += (num_str[i] - L'0');
    }
    return num;
}


wchar_t ReadUnicode::getProgmWStr(const size_t nowPtr){
    return progm_w_str[nowPtr];
}

bool ReadUnicode::isEmpty(){
    return progm_w_str.empty();
}

void ReadUnicode::InitReadUnicode(){
    progm_w_str.clear();
}

// 读取 UTF8 文件, 返回Unicode（UCS-2）字符串
void ReadUnicode::readFile2USC2(const string filename)
{
    // 打开文件
    wcout<<filename.c_str()<<endl;

    file.open(filename);
    if (!file.is_open()) {
        wcout << L"cannot open file!" << endl;
        return;
    }
    wcout << L"[Compiling file '" << filename.c_str() << L"]" << endl;

    // 禁止过滤空白符
    file >> noskipws;
    // 跳过 UTF8 BOM（0xEFBBBF）
    if (file.get() != 0xEF || file.get() != 0xBB || file.get() != 0xBF) {
        file.seekg(0, ios::beg);
    }

    unsigned char B; // 1字节
    wchar_t wchar; // 2字节存储UCS-2码点
    wstring w_str(L""); // 用于存储转换结果的 Unicode 码点序列
    int len; // 单个 UTF8 字符的编码长度

    while ((file >> B) && !file.eof()) {
        // 单字节编码 0xxxxxxx
        if (B < 0b10000000) {
            wchar = B;
        } // 多字节编码，获取编码长度
        else {
            // 超出可用 Unicode 范围
            if (B > 0b11110100) {
                wcout << L"Invalid unicode range" << endl;
                return;
            } // 4字节编码 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            else if (B >= 0b11110000) {
                len = 4;
            }
            // 1110xxxx 10xxxxxx 10xxxxxx
            else if (B >= 0b11100000) {
                len = 3;
            }
            // 110xxxxx 10xxxxxx
            else if (B >= 0b11000000) {
                len = 2;
            } else {
                // 除单字节外，首字节不能小于 0b11000000
                wcout << L"Invalid utf8 leading code" << endl;
                return;
            }
            // 通过左移再右移的方法去掉首字节中的 UTF8 标记
            B = B << (len + 1);
            wchar = B >> (len + 1);
            // 处理后续字节
            while (len > 1) {
                B = file.get();
                // 如果 f 到达 eof，则 c 会返回 255
                // 后续编码必须是 0b10xxxxxx 格式
                if (B >= 0b11000000) {
                    wcout << L"Invalid utf8 tailing code" << endl;
                    return;
                }
                len--;
                B = B & 0b00111111; // 去掉 UTF8 标记
                wchar = wchar << 6; // 腾出 6 个 bit 的位置
                wchar += B; // 将去掉了 UTF8 标记的编码合并进来
            }
        }
        // 存储解解析结果
        w_str.push_back(wchar);
    }
    w_str.push_back(L'#');
    progm_w_str = w_str;
    file.clear();
    // file.seekg(0, file.beg);
    file.close();
}

ReadUnicode readUnicode;