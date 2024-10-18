#include <lexer.hpp>

// 判断是否为数字
bool IsDigit(wchar_t ch){
    if (ch >= L'0' && ch <= L'9')
        return true;
    else
        return false;
}

// 判断是否为字母
bool IsLetter(wchar_t ch){
    if ((ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z'))
        return true;
    else
        return false;
}

// 判断是否为终止符
bool IsBoundary(wchar_t ch){
    return ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'#' || ch == L'\0' || ch == L';' || ch == L',';
}

//判断是否是操作符
int IsOperator(wchar_t ch){
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
void GetChar(){
    ch = progm_w_str[nowPtr++];
    colPos++;
}

//使当前指针跳过连续空格到下一个非空格处
void GetBC(){
    while (progm_w_str[nowPtr] && (progm_w_str[nowPtr] == L' ' || progm_w_str[nowPtr] == L'\t')) {
        GetChar();
    }
}

void Retract(){
    ch=progm_w_str[--nowPtr];
    colPos--;
}

void Concat(){
    strToken+=ch;
}

int Reserve(){
    for (int i = 0; i < RSV_WORD_MAX; i++) {
        if (resv_table[i] == strToken) {
            return i;
        }
    }
    return -1;
}

void GetWord(){
    //更新上一个匹配到的合法字符的位置信息
    if (ch != L'\n') {
        preWordCol = colPos;
        preWordRow = rowPos;
    }

    //找到第一个不是 的ch，可能是\0或\n或结束符
    strToken.clear();
    GetChar();
    GetBC();
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
    }else if(IsLetter(ch)){
        Concat();
        GetChar();
        while(IsLetter(ch)||IsDigit(ch)){
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
    }else if(IsDigit(ch)){
        Concat();
        GetChar();
        while(IsDigit(ch)){
            Concat();
            GetChar();
        }
        if(IsLetter(ch)){
            //error
            error(ILLEGAL_WORD, (L"'" + strToken + L"'").c_str());
            //到下一个界符
            while(!IsBoundary(ch)){
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
            error(MISSING, L"'='");
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
            // preWordCol++;
        }else{
            tokenType=GRT;
            Retract();
        }
    }else{
        int code = IsOperator(ch);
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
            error(ILLEGAL_WORD, (L"'" + strToken + L"'").c_str());
            tokenType=NUL;
        }
    }
}