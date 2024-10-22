#include <lexer.hpp>

void Lexer::InitLexer()
{
    // 变量初始化
    ch = L' '; // 用于词法分析器，存放最近一次从文件中读出的字符
    tokenType = NUL; // 最近一次识别出来的 token 的类型
    strToken = L""; // 最近一次识别出来的标识符的名字
    colPos = 0; // 列指针
    rowPos = 1; // 行指针
    preWordCol = 0; // 上一个非空白合法词尾列指针
    preWordRow = 1; // 上一个非空白合法词行指针
    nowPtr = 0;

    // 符号名表初始化
    sym_map[NUL] = L"NUL";
    sym_map[IDENT] = L"IDENT";
    sym_map[NUMBER] = L"NUMBER";
    sym_map[PLUS] = L"PLUS";
    sym_map[MINUS] = L"MINUS";
    sym_map[MULTI] = L"MULTI";
    sym_map[DIVIS] = L"DIVIS";
    sym_map[ODD_SYM] = L"ODD_SYM";
    sym_map[EQL] = L"EQL";
    sym_map[NEQ] = L"NEQ";
    sym_map[LSS] = L"LSS";
    sym_map[LEQ] = L"LEQ";
    sym_map[GRT] = L"GRT";
    sym_map[GEQ] = L"GEQ";
    sym_map[LPAREN] = L"LPAREN";
    sym_map[RPAREN] = L"RPAREN";
    sym_map[COMMA] = L"COMMA";
    sym_map[SEMICOLON] = L"SEMICOLON";
    sym_map[ASSIGN] = L"BECOMES";
    sym_map[BEGIN_SYM] = L"BEGIN_SYM";
    sym_map[END_SYM] = L"END_SYM";
    sym_map[IF_SYM] = L"IF_SYM";
    sym_map[THEN_SYM] = L"THEN_SYM";
    sym_map[WHILE_SYM] = L"WHILE_SYM";
    sym_map[DO_SYM] = L"DO_SYM";
    sym_map[CALL_SYM] = L"CALL_SYM";
    sym_map[CONST_SYM] = L"CONST_SYM";
    sym_map[VAR_SYM] = L"VAR_SYM";
    sym_map[PROC_SYM] = L"PROC_SYM";
    sym_map[WRITE_SYM] = L"WRITE_SYM";
    sym_map[READ_SYM] = L"READ_SYM";
    sym_map[PROGM_SYM] = L"PROGM_SYM";
    sym_map[ELSE_SYM] = L"ELSE_SYM";
}

// 判断是否为数字
bool Lexer::IsDigit(){
    if (ch >= L'0' && ch <= L'9')
        return true;
    else
        return false;
}

// 判断是否为字母
bool Lexer::IsLetter(){
    if ((ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z'))
        return true;
    else
        return false;
}

// 判断是否为终止符
bool Lexer::IsBoundary(){
    return ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'#' || ch == L'\0' || ch == L';' || ch == L',';
}

//判断是否是操作符
int Lexer::IsOperator(){
    wchar_t* p=opr_table;
    while(p-opr_table<OPR_MAX){
        if(ch==*p){
            return p-opr_table;
        }
        p++;
    }
    return -1;
}

//将下一个字符读到ch，搜索指示器前移一个字符位置
void Lexer::GetChar(){
    ch = readUnicode.getProgmWStr(nowPtr);
    nowPtr++;
    colPos++;
}

//使当前指针跳过连续空格到下一个非空格处
void Lexer::GetBC(){
    while (readUnicode.getProgmWStr(nowPtr) && (readUnicode.getProgmWStr(nowPtr) == L' ' || readUnicode.getProgmWStr(nowPtr) == L'\t')) {
        GetChar();
    }
}

void Lexer::Retract(){
    nowPtr--;
    ch=readUnicode.getProgmWStr(nowPtr);
    colPos--;
}

void Lexer::Concat(){
    strToken+=ch;
}

int Lexer::Reserve(){
    for (int i = 0; i < RSV_WORD_MAX; i++) {
        if (resv_table[i] == strToken) {
            return i;
        }
    }
    return -1;
}

void Lexer::GetWord(){
    //更新上一个匹配到的合法字符的位置信息
    if (ch != L'\n') {
        preWordCol = colPos;
        preWordRow = rowPos;
    }

    //找到第一个不是 的ch，可能是\0或\n或结束符
    strToken.clear();
    GetBC();
    GetChar();

    if(ch==L'\0'){
        return;
    }
    if(ch==L'\n'){
        //连续跳过空行，同时保证pre指向是合法字符不更新
        colPos=0;
        rowPos++;
        GetWord();
        return;
    }else if(ch==L'#'){
        Concat();
        tokenType=NUL;
    }else if(IsLetter()){
        Concat();
        GetChar();
        while(IsLetter()||IsDigit()){
            Concat();
            GetChar();
        }

        int code=Reserve();
        switch (code) {
        case -1:
            tokenType = IDENT;
            break;
        case 0:
            tokenType = ODD_SYM;
            break;
        case 1:
            tokenType = BEGIN_SYM;
            break;
        case 2:
            tokenType = END_SYM;
            break;
        case 3:
            tokenType = IF_SYM;
            break;
        case 4:
            tokenType = THEN_SYM;
            break;
        case 5:
            tokenType = WHILE_SYM;
            break;
        case 6:
            tokenType = DO_SYM;
            break;
        case 7:
            tokenType = CALL_SYM;
            break;
        case 8:
            tokenType = CONST_SYM;
            break;
        case 9:
            tokenType = VAR_SYM;
            break;
        case 10:
            tokenType = PROC_SYM;
            break;
        case 11:
            tokenType = WRITE_SYM;
            break;
        case 12:
            tokenType = READ_SYM;
            break;
        case 13:
            tokenType = PROGM_SYM;
            break;
        case 14:
            tokenType = ELSE_SYM;
            break;
        default:
            tokenType = NUL;
            break;
        }
        Retract();
    }else if(IsDigit()){
        Concat();
        GetChar();
        while(IsDigit()){
            Concat();
            GetChar();
        }
        if(IsLetter()){
            //error
            errorHandle.error(ILLEGAL_WORD, (L"'" + strToken + L"'").c_str(),preWordRow,preWordCol,rowPos,colPos);
            //到下一个界符
            while(!IsBoundary()){
                GetChar();
            }
            Retract();
            strToken.clear();
            tokenType=NUL;
        }else{//遇到界符
            tokenType=NUMBER;
            Retract();           
        }
    }else if(ch==L':'){
        Concat();
        GetChar();
        if(ch==L'='){
            Concat();
            // preWordCol++;
            tokenType=ASSIGN;
        }else{
            //error
            // error(MISSING, L"'='");
            errorHandle.error(MISSING, L"'='",preWordRow,preWordCol,rowPos,colPos);
            strToken.clear();
            tokenType=NUL;
        }
    }else if(ch==L'<'){
        Concat();
        GetChar();
        if(ch==L'='){
            Concat();
            tokenType=LEQ;
            // preWordCol++;
        }else if(ch==L'>'){
            Concat();
            tokenType=NEQ;
            // preWordCol++;
        }else{
            tokenType=LSS;
            Retract();
        }
    }else if(ch==L'>'){
        Concat();
        GetChar();
        if(ch==L'='){
            Concat();
            tokenType=GEQ;
            preWordCol++;
        }else{
            tokenType=GRT;
            Retract();
        }
    }else{
        int code = IsOperator();
        if (code != -1) {
            Concat();
            switch (code) {
            case 0:
                tokenType = PLUS;
                break;
            case 1:
                tokenType = MINUS;
                break;
            case 2:
                tokenType = MULTI;
                break;
            case 3:
                tokenType = DIVIS;
                break;
            case 4:
                tokenType = EQL;
                break;
            case 7:
                tokenType = LPAREN;
                break;
            case 8:
                tokenType = RPAREN;
                break;
            case 9:
                tokenType = COMMA;
                break;
            case 10:
                tokenType = SEMICOLON;
                break;
            default:
                break;
            }
        }else{
            Concat();
            //error
            // error(ILLEGAL_WORD, (L"'" + strToken + L"'").c_str());
            errorHandle.error(ILLEGAL_WORD, (L"'" + strToken + L"'").c_str(),preWordRow,preWordCol,rowPos,colPos);
            tokenType=NUL;
        }
    }
}

wchar_t Lexer:: GetCh(){
    return ch;
}

Lexer lexer;