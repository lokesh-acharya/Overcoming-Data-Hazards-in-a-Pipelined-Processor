#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <iomanip>

using namespace std;

vector<string> v;
int noOfInstructions = 0;
int cycles = 0;
int PCcopy = 0;
int cache[2] = {-1, -1};
bool stall = false;
bool forwrd = false;
string prevProcess, stalledProcess;
string aluResult, lwResult;
int fwdvalue, fwdreg;

void IF(vector<string> v, int &PC, int *regs, int *memo);
void ID(string inst, int &PC, int *regs, int *memo);
void EX(string op, int src1, int src2, int src3, int &PC, int *regs, int *memo);
void MEM(string op,int addr, int src, int &PC, int *regs, int *memo);
void WB(int wvalue, int wreg, int &PC, int *regs, int *memo);

string aluop(int fcode){
    switch(fcode){
case 0:
    return "sll";
    break;
case 2:
    return "srl";
    break;
case 32:
    return "add";
    break;
case 34:
    return "sub";
    break;
default:
    return "other";
    }
}

int power(int x, int y){
    int r=1;
    for(int i=0; i<y;i++){
        r=x*r;
    }
    return r;
}

void IF(vector<string> v, int &PC, int *regs, int *memo) {
    cout<<"\nProgram Counter: "<<PC<<endl;
    if(::stall == false) ::cycles++;
    cout<<"\nIF\n"<<endl;
    if(PC < ::noOfInstructions) {
    string inst = v.at(PC);
    cout<<inst<<endl;
    ID(inst, PC, regs, memo);
    }
}

void ID(string inst, int &PC, int *regs, int *memo){
    cout<<"\nID\n"<<endl;

    string op;
    int opcode = stoi(inst.substr (0,6), nullptr, 2);
    int rs = stoi(inst.substr (6,5), nullptr, 2);
    int rt = stoi(inst.substr (11,5), nullptr, 2);
    int rd = stoi(inst.substr (16,5), nullptr, 2);
    int shamt = stoi(inst.substr (21,5), nullptr, 2);
    int off = stoi(inst.substr (16,16), nullptr, 2);
    int addr = stoi(inst.substr (6,26), nullptr, 2);
    int fcode = stoi(inst.substr (26,6), nullptr, 2);

    ::cache[0] = ::cache[1];
    switch(opcode) {
        case 35:
            ::cache[1] = rt;
            if(::cache[0] == rs) {
                    ::fwdreg = rs;
                    if(::prevProcess == "lw") {::stall = true; ::prevProcess = "lw";}
                    else if(::prevProcess == "alu") ::forwrd =true;
            }
        break;
        case 0:
            ::cache[1] = rd;
            if(::cache[0] == rs||::cache[0] == rt) {
                if(::cache[0] == rs) ::fwdreg = rs; else ::fwdreg = rt;
                if(::prevProcess == "lw") {::stall = true; ::prevProcess = aluop(fcode);}
                else if(::prevProcess == "alu") ::forwrd =true;
            }
        break;
        case 43:
            ::cache[1] = -1;
            if(::cache[0] == rs||::cache[0] == rt) {
                if(::cache[0] == rs) ::fwdreg = rs; else ::fwdreg = rt;
                if(::prevProcess == "lw") {::stall = true; ::prevProcess = "sw";}
                else if(::prevProcess == "alu") ::forwrd =true;
            }
        break;
        case 8:
            ::cache[1] = -1;
            if(::cache[0] == addr) {
                ::fwdreg = addr;
                if(::prevProcess == "lw") {::stall = true; ::prevProcess = "jr";}
                else if(::prevProcess == "alu") ::forwrd =true;
            }
        break;
        case (4 || 5):
            ::cache[1] = -1;
            if(::cache[0] == rs || ::cache[0] ==rt) {
                if(::cache[0] == rs) ::fwdreg = rs; else ::fwdreg = rt;
                if(::prevProcess == "lw") {::stall = true; if(opcode==4) ::prevProcess ="beq"; else ::prevProcess ="bne";}
                else if(::prevProcess == "alu") ::forwrd =true;
            }
        break;
        case (6):
            ::cache[1] = -1;
            if(::cache[0] == rs || ::cache[0] ==rt) {
                if(::cache[0] == rs) ::fwdreg = rs; else ::fwdreg = rt;
                if(::prevProcess == "lw") {::stall = true; ::prevProcess ="blez";}
                else if(::prevProcess == "alu") ::forwrd =true;
            }
        break;
        case (7):
            ::cache[1] = -1;
            if(::cache[0] == rs || ::cache[0] ==rt) {
                if(::cache[0] == rs) ::fwdreg = rs; else ::fwdreg = rt;
                if(::prevProcess == "lw") {::stall = true; ::prevProcess ="bgtz";}
                else if(::prevProcess == "alu") ::forwrd =true;
            }
        break;

        default:
        break;
    }

    cout<<"Last Modify: "<<(::cache[0])<<" Present Modify: "<<(::cache[1])<<endl;
    cout<<"Stall Value: "<<(::stall)<<endl;
    cout<<"Forward Value: "<<(::forwrd)<<endl;

    if((::stall == true) || PC == (noOfInstructions-1)) ::cycles++;
    if(::stall == false) {
        if(opcode == 0) {
            cout<<fcode<<endl;
            if(fcode == 32 || fcode == 34) {
                if(fcode == 32) op= "add"; else op= "sub";
                ::prevProcess = "alu";
                cout<<op<<endl;
                EX(op, rs, rt, rd, PC, regs, memo);
            }
            else if(fcode == 0 || fcode == 2) {
                if(fcode == 0) op= "sll"; else op= "srl";
                ::prevProcess = "alu";
                cout<<op<<endl;
                EX(op, shamt, rt, rd, PC, regs, memo);
            }
        }
        else if(opcode == 43 || opcode == 35){
            if(opcode == 43) op = "sw"; else op = "lw";
            ::prevProcess = op;
            cout<<op<<endl;
            EX(op, rs, rt, off, PC, regs, memo);
        }
        else if(opcode == 2 || opcode == 3 || opcode == 8) {
            if(opcode == 2) op = "j"; else if(opcode == 3) op = "jal"; else op = "jr";
            ::prevProcess = op;
            cout<<op<<endl;
            EX(op, addr, 0, 0, PC, regs, memo);
        }
        else if(opcode == 4 || opcode == 5){
            if(opcode == 4) op = "beq"; else op = "bne";
            ::prevProcess = op;
            cout<<op<<endl;
            EX(op, rs, rt, off, PC, regs, memo);
        }
        else if(opcode == 6 || opcode == 7){
            if(opcode == 6) op = "blez"; else op = "bgtz";
            ::prevProcess = op;
            cout<<op<<endl;
            EX(op, rs, 0, off, PC, regs, memo);
        }
    }
    else {
        //while(::stall == true){
        //    cout<<"waiting"<<endl;
        //
        op = "stall";
        ::cycles = ::cycles - 1;
        if(::prevProcess == "add" ||::prevProcess == "sub") EX(op, rs, rt, rd, PC, regs, memo);
        else if(::prevProcess == "sll" ||::prevProcess == "srl") EX(op, shamt, rt, rd, PC, regs, memo);
        else if(::prevProcess == "sw" ||::prevProcess == "lw"||::prevProcess == "bne" ||::prevProcess == "beq"||::prevProcess == "bgtz" ||::prevProcess == "blez") EX(op, rs, rt, off, PC, regs, memo);
        else if(::prevProcess == "j" ||::prevProcess == "jr"||::prevProcess == "jal") EX(op, addr, 0, 0, PC, regs, memo);
    }
}

void EX(string op, int src1, int src2, int src3, int &PC, int *regs, int *memo) {
    if((::stall == true) || PC == (noOfInstructions-1)) ::cycles++;

    cout<<"\nEX\n"<<endl;
    cout<<"Forward: "<<(::forwrd)<<"\n"<<endl;
    cout<<"Forward Value: "<<(::fwdvalue)<<"\n"<<endl;

    int result,addr;

    if(op == "add"){
        if(::forwrd==true){
            if(::fwdreg==src1 && ::fwdreg==src2) result = 2*fwdvalue;
            else if(::fwdreg==src1) result = fwdvalue + regs[src2];
            else if(::fwdreg==src2) result = fwdvalue + regs[src1];
            ::forwrd = false;
        }
        else result = regs[src1] + regs[src2];
        ::fwdvalue = result;
        cout<<"rs: "<<src1<<" ; rt: "<<src2<<" ; rd: "<<src3<<endl;
        cout<<"result: "<<result<<endl;
        WB(result, src3, PC, regs, memo);
    }
    else if(op == "sub"){
        if(::forwrd==true){
            if(::fwdreg==src1 && ::fwdreg==src2) result = 0;
            else if(::fwdreg==src1) result = fwdvalue - regs[src2];
            else if(::fwdreg==src2) result = regs[src1] - fwdvalue;
            ::forwrd = false;
        }
        else result = regs[src1] - regs[src2];
        ::fwdvalue = result;
        cout<<"rs: "<<src1<<" ; rt: "<<src2<<" ; rd: "<<src3<<endl;
        cout<<"result: "<<result<<endl;
        WB(result, src3, PC, regs, memo);
    }
    else if(op == "sll"){
        if(::forwrd==true){
            if(::fwdreg==src2) result = fwdvalue*power(2,src1);
            ::forwrd = false;
        }
        else result = regs[src2]*power(2,src1);
        ::fwdvalue = result;
        cout<<"shamt: "<<src1<<" ; rt: "<<src2<<" ; rd: "<<src3<<endl;
        cout<<"result: "<<result<<endl;
        WB(result, src3, PC, regs, memo);
    }
    else if(op == "srl"){
        if(::forwrd==true){
            if(::fwdreg==src2) result = fwdvalue/power(2,src1);
            ::forwrd = false;
        }
        else result = regs[src2]/power(2,src1);
        ::fwdvalue = result;
        cout<<"shamt: "<<src1<<" ; rt: "<<src2<<" ; rd: "<<src3<<endl;
        cout<<"result: "<<result<<endl;
        WB(result, src3, PC, regs, memo);
    }
    else if(op == "sw"){
        if(::forwrd==true){
            if(::fwdreg == src1) addr = fwdvalue + (src3/4);
        }
        addr = regs[src1] + (src3/4);
        cout<<regs[18]<<" "<<src3<<endl;
        cout<<"rs: "<<src1<<" ; rt: "<<src2<<" ; off: "<<src3<<endl;
        cout<<"addr: "<<addr<<endl;
        MEM(op, addr, src2, PC, regs, memo);
    }
    else if(op == "lw"){
        if(::forwrd==true){
            if(::fwdreg == src1) addr = fwdvalue + (src3/4);
            ::forwrd = false;
        }
        else addr = regs[src1] + (src3/4);
        cout<<"rs: "<<src1<<" ; rt: "<<src2<<" ; off: "<<src3<<endl;
        cout<<"addr: "<<addr<<endl;
        MEM(op, addr, src2, PC, regs, memo);
    }
    else if(op == "beq"){
        if(::forwrd==true){
            if(::fwdreg==src1 && ::fwdreg==src2) result = 0;
            else if(::fwdreg==src1) result = fwdvalue - regs[src2];
            else if(::fwdreg==src2) result = regs[src1] - fwdvalue;
            ::forwrd = false;
        }
        else result = regs[src1] - regs[src2];
        addr = src3;
        cout<<"result: "<<result<<endl;
        MEM(op, addr, result, PC, regs, memo);
    }
    else if(op == "bne"){
        if(::forwrd==true){
            if(::fwdreg==src1 && ::fwdreg==src2) result = 0;
            else if(::fwdreg==src1) result = fwdvalue - regs[src2];
            else if(::fwdreg==src2) result = regs[src1] - fwdvalue;
            ::forwrd = false;
        }
        else result = regs[src1] - regs[src2];
        addr = src3;
        cout<<"result: "<<result<<endl;
        MEM(op, addr, result, PC, regs, memo);
    }
    else if(op == "blez" || op == "bgtz"){
        if(::forwrd==true){
            if(::fwdreg== src1) result = fwdvalue;
            ::forwrd = false;
        }
        else result = regs[src1];
        addr = src3;
        cout<<"result: "<<result<<endl;
        MEM(op, addr, result, PC, regs, memo);
    }
    else if(op == "j"){
        PC = src1;
        cout<<PC<<endl;
        IF(::v, PC, regs, memo);
    }
    else if(op == "jal"){
        regs[31] = PC+1;
        cout<<"return addr: "<<PC<<endl;
        PC = src1;
        cout<<PC<<endl;
        IF(::v, PC, regs, memo);
    }
    else if(op == "jr"){
        if(::forwrd==true){
            if(::fwdreg== src1) PC = fwdvalue;
            ::forwrd = false;
        }
        else PC = regs[src1];
        cout<<PC<<endl;
        IF(::v, PC, regs, memo);
    }
    else if(op == "stall"){
        cout<<"WAITING!!"<<endl;
        ::stall = false;
        ::forwrd = true;
        op = ::prevProcess;
        EX(op, src1, src2, src3, PC, regs, memo);
    }
}

void MEM(string op,int addr, int src, int &PC, int *regs, int *memo){
    if((::stall == true) || PC == (noOfInstructions-1)) ::cycles++;

    cout<<"\nMEM\n"<<endl;

    int data;
    if(op == "sw"){
        if(::forwrd==true){
            if(src == ::fwdreg) memo[addr] = fwdvalue;
            ::forwrd = false;
        }
        else memo[addr] = regs[src];
        cout<<memo[addr]<<endl;
        PC++;
        ::stall = false;
        IF(::v, PC, regs, memo);
    }
    else if(op == "lw"){
        data = memo[addr];
        ::fwdvalue = data;
        cout<<data<<endl;
        WB(data, src, PC, regs, memo);
    }
    else if(op == "beq"){
        if(src == 0) {PC = addr; cout<<PC<<endl; IF(::v, PC, regs, memo);}
        else {PC++; cout<<PC<<endl; ::stall = false; IF(::v, PC, regs, memo);}
    }
    else if(op == "bne"){
        if(src != 0) {PC = addr; cout<<PC<<endl; IF(::v, PC, regs, memo);}
        else {PC++; cout<<PC<<endl; ::stall = false; IF(::v, PC, regs, memo);}
    }
    else if(op == "blez"){
        if(src <= 0) {PC = addr; cout<<PC<<endl; IF(::v, PC, regs, memo);}
        else {PC++; cout<<PC<<endl; ::stall = false; IF(::v, PC, regs, memo);}
    }
    else if(op == "bgtz"){
        if(src > 0) {PC = addr; cout<<PC<<endl; IF(::v, PC, regs, memo);}
        else {PC++; cout<<PC<<endl; ::stall = false; IF(::v, PC, regs, memo);}
    }
    //else if(op == "stall"){
    //    cout<<"WAITING!!"<<endl;
    //    else if(::prevProcess == "lw"){
    //        WB(-1, 0, PC, regs, memo);
    //    }
    //
}

void WB(int wvalue, int wreg, int &PC, int *regs, int *memo){
    if(PC == (noOfInstructions-1)) ::cycles++;

    cout<<"\nWB\n"<<endl;

    if(wvalue == (-1)) {
        cout<<"WAITING!!"<<endl;
        ::stall = false;
        ::cache[1] = -1;
        IF(::v, PC, regs, memo);
    }

    regs[wreg] = wvalue;
    cout<<"write value: "<<wvalue<<endl;
    PC++;
    ::stall = false;
    IF(::v, PC, regs, memo);
}

int fillInstructions(vector<string> &inst){
    ifstream myfile ("t2");
    int inum=0;
    string line;
    int i=0;
    while ( getline (myfile,line)) {
        if(line == "00000000000000000000000000000000") return inum;
        inum++;
        inst.push_back(line);
        cout<<inst[i]<<endl;
        i++;
    }
    return inum;
}

int fillInstructions1(vector<string> &inst){
    cout<<"fillInstructions main aaya!"<<endl;
    string data[4];
    string line;
    for(int i = 0; i < 4; i++){
                  cin >> line;
                  inst.push_back(line);
                  cout<<inst.at(i)<<endl;
    }
    return 4;
}

int main(){
    int PC = 0;
    int regs[32];
    int memo[4000];

    for(int j=0;j<32;j++){
    regs[j]=0;
    }

    for(int j=0;j<4000;j++){
    memo[j]=0;
    }

    regs[0]= 1001;
    regs[1]= 2001;
    regs[3]= 100;
    regs[4]= 200;
    regs[6]= 11;
    regs[8]= 1052;
    regs[10] = 999;
    regs[11] = 12345;
    regs[12] = 999;
    regs[17] = 0;
    regs[18] = 1;
    regs[19] = 2;
    regs[24]=regs[25]=1;
    regs[26]=regs[27]=2;

    memo[0] = -10;
    memo[1] = 13;
    memo[2] = 118;

    ::noOfInstructions = fillInstructions(::v);
    cout<<(::v.size())<<endl;

    if(::noOfInstructions == 0) {cout<<"No instructions are present"<<endl; return 0;}
    else {
        ::PCcopy = PC;
        IF(::v, PC, regs, memo);

        float IPC = (::noOfInstructions/(cycles-1));
        cout<<(::noOfInstructions)<<endl;
        cout<<"Program completed!!\nNo. of cycles taken: "<<(::cycles-1)<<"\nIPC: "<<(::noOfInstructions)<<"/"<<(::cycles-1)<<endl;
        return 0;
    }
}
