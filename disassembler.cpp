#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
using namespace std;

// CS2323 Lab-3: RISC-V Disassembler
// Submission by Edward Nathan Varghese - CS22BTECH11020

int PC = 0;
map <int, bool> labelled;
map <int, int> PC_to_label;

// Function Prototypes
string HexToBinary (string, map <char, string>&);
void initializeMap (map <char, string>&);
string BinToAssembly (string, string);
string BinToReg (string);
string BinToImm (string, bool);
int BinToImmB (string, bool);
int BinToImmJ (string, bool);

int main (void) {
    freopen ("input.txt", "r", stdin);
    freopen ("output.txt", "w", stdout);
    ifstream read ("input.txt");

    map <char, string> binmap;
    initializeMap (binmap);

    string s, bin;
    vector <string> lines;

    // Reading lines and passing it to BinToAssembly
    while (read >> s){
        if (!s.length()) break;
        bin = HexToBinary (s, binmap);
        if (!labelled[PC]) labelled[PC] = false;
        lines.push_back(BinToAssembly (bin, s));
        PC += 4;
    }

    // Assigning labels
    map <int, string> text_label;
    int label_counter = 1;
    for (int pc = 0; pc < PC; pc += 4){
        if (labelled[pc]){        
            text_label[pc] = "L" + to_string(label_counter);
            label_counter++;
            lines[pc/4] = text_label[pc] + ": " + lines[pc/4];
        }
    }

    // Printing out the lines
    for (int pc = 0; pc < PC; pc += 4){
        if (PC_to_label[pc]){
            if (PC_to_label[pc] - 4 <= 0 || PC_to_label[pc] - 4 >= PC);
            else{
                for (int i = lines[pc/4].length() - 1; i > -1; i--){ // deleting the offset
                    if (lines[pc/4][i] == ' '){
                        lines[pc/4] = lines[pc/4].substr(0, i + 1);
                        break;
                    }
                }
                lines[pc/4] += text_label[PC_to_label[pc] - 4];
            }
        }
        cout << lines[pc/4] << '\n';
    }
    return 0;
}

// Converts the given binary encoding to an assembly instruction
string BinToAssembly (string bin, string hex){
    string opcode = bin.substr(25, 7);

    string ins = "", invalid = "invalid";

    // For R-format Instructions, the opcode is 0110011
    if(opcode == "0110011"){
        string rs1 = bin.substr(12, 5), rs2 = bin.substr(7, 5), funct7 = bin.substr(0, 7);
        string funct3 = bin.substr(17, 3), rd = bin.substr(20, 5);

        if (funct7 == "0000000"){
            if (funct3 == "000") ins += "add ";
            else if (funct3 == "100") ins += "xor ";
            else if (funct3 == "110") ins += "or ";
            else if (funct3 == "111") ins += "and ";
            else if (funct3 == "001") ins += "sll ";
            else if (funct3 == "101") ins += "srl ";
            else if (funct3 == "010") ins += "slt ";
            else if (funct3 == "011") ins += "sltu ";
            else return invalid;
        }
        else if (funct7 == "0100000"){
            if (funct3 == "000") ins += "sub ";
            else if (funct3 == "101") ins += "sra ";
            else return invalid;
        }
        else return invalid;

        ins += BinToReg (rd) + ", ";
        ins += BinToReg (rs1) + ", ";
        ins += BinToReg (rs2);
        return ins;
    }

    // For I-format Instructions, the opcodes are 0010011, 0000011 and 1100111
    else if(opcode == "0010011" || opcode == "0000011" || opcode == "1100111"){
        string rs1 = bin.substr(12, 5), imm = bin.substr(0, 12);
        string funct3 = bin.substr(17, 3), rd = bin.substr(20, 5);
        
        bool sign = true;
        if(opcode == "0010011"){
            if (funct3 == "000") ins += "addi ";
            else if (funct3 == "100") ins += "xori ";
            else if (funct3 == "110") ins += "ori ";
            else if (funct3 == "111") ins += "andi ";
            else if (funct3 == "001") ins += "slli ";
            else if (funct3 == "101"){
                if (bin.substr(0, 6) == "000000") ins += "srli ";
                else ins += "srai ";
            }
            else if (funct3 == "010") ins += "slti ";
            else if (funct3 == "011"){
                ins += "sltiu ";
                sign = false;
            }
            else return invalid;
        }

        else if(opcode == "0000011"){
            sign = true;
            if (funct3 == "000") ins += "lb ";
            else if (funct3 == "001") ins += "lh ";
            else if (funct3 == "010") ins += "lw ";
            else if (funct3 == "011") ins += "ld ";
            else if (funct3 == "100"){
                ins += "lbu ";
                sign = false;
            }
            else if (funct3 == "101"){
                ins += "lhu ";
                sign = false;
            }
            else if (funct3 == "110"){
                ins += "lwu ";
                sign = false;
            }
            else return invalid;

            ins += BinToReg (rd) + ", ";
            ins += BinToImm (imm, sign);
            ins += "(" + BinToReg (rs1) + ")";
            return ins;
        }

        else if(opcode == "1100111"){ // jalr
            if (funct3 == "000") ins += "jalr ";
            else return invalid;
        }

        ins += BinToReg (rd) + ", ";
        ins += BinToReg (rs1) + ", ";
        ins += BinToImm (imm, sign);
        return ins;
    }

    // For S-format Instructions, the opcode is 0100011
    else if(opcode == "0100011"){
        string rs1 = bin.substr(12, 5), rs2 = bin.substr(7, 5);
        string funct3 = bin.substr(17, 3), imm =  bin.substr(0, 7) + bin.substr(20, 5);

        if (funct3 == "000") ins += "sb ";
        else if (funct3 == "001") ins += "sh ";
        else if (funct3 == "010") ins += "sw ";
        else if (funct3 == "011") ins += "sd ";
        else return invalid;

        ins += BinToReg (rs2) + ", ";
        ins += BinToImm (imm, true);
        ins += "(" + BinToReg (rs1) + ")";
        return ins;
    }

    // For B-format Instructions, the opcode is 1100011
    else if(opcode == "1100011"){
        string rs1 = bin.substr(12, 5), rs2 = bin.substr(7, 5);
        string funct3 = bin.substr(17, 3);
        string imm =  bin.substr(0, 1) + bin.substr(24, 1) + bin.substr(1, 6) + bin.substr(20, 4);

        bool sign = true;
        if (funct3 == "000") ins += "beq ";
        else if (funct3 == "001") ins += "bne ";
        else if (funct3 == "100") ins += "blt ";
        else if (funct3 == "101"){
            ins += "bge ";
            sign = false;
        }
        else if (funct3 == "110"){
            ins += "bltu ";
            sign = false;
        }
        else if (funct3 == "111"){
            ins += "bgeu ";
            sign = false;
        }
        else return invalid;

        if (PC + BinToImmB (imm, sign) > -1) labelled[PC + BinToImmB (imm, sign)] = true;
        PC_to_label[PC] = PC + BinToImmB (imm, true) + 4;

        ins += BinToReg (rs1) + ", ";
        ins += BinToReg (rs2) + ", ";
        ins += to_string(BinToImmB (imm, sign));
        return ins;
    }

    // For J-format Instructions (jal), the opcode is 1101111
    else if(opcode == "1101111"){
        string rd = bin.substr(20, 5);
        string imm =  bin.substr(0, 1) + bin.substr(12, 8) + bin.substr(11, 1) + bin.substr(1, 10);

        if (PC + BinToImmJ (imm, true) > -1) labelled[PC + BinToImmJ (imm, true)] = true;
        PC_to_label[PC] = PC + BinToImmJ (imm, true) + 4;

        ins += "jal ";
        ins += BinToReg (rd) + ", ";
        ins += to_string(BinToImmJ (imm, true));
        return ins;
    }

    // For U-format Instructions (lui), the opcode is 0110111
    else if(opcode == "0110111"){
        string rd = bin.substr(20, 5);
        for (int i = 0; i < 5; i++) hex[i] = tolower (hex[i]);

        ins += "lui ";
        ins += BinToReg (rd) + ", ";
        ins += "0x" + hex.substr(0, 5);
        return ins;
    }

    else return invalid;
}


// Converts a 5 digit binary to its respective register
string BinToReg (string s){
    int num = 0, x = 16;
    for (int i = 0; i < 5; i++){
        if (s[i] == '1') num += x;
        x /= 2;
    }
    return "x" + to_string(num);
}

// Converts Binary to Immediate
string BinToImm (string s, bool sign){
    int num = 0, x = 2048;
    for (int i = 0; i < 12; i++){
        if (s[i] == '1') num += x;
        x /= 2;
    }
    if (sign && num > 2047) num = 2047 - num;
    return to_string(num);
}

// Converts Binary to Immediate
int BinToImmB (string s, bool sign){
    int num = 0, x = 4096;
    for (int i = 0; i < 12; i++){
        if (s[i] == '1') num += x;
        x /= 2;
    }
    if (sign && num > 4095) num = 4095 - num;
    return num;
}

// Converts Binary to Immediate
int BinToImmJ (string s, bool sign){
    int num = 0, x = 1048576;
    for (int i = 0; i < 20; i++){
        if (s[i] == '1') num += x;
        x /= 2;
    }
    if (sign && num > 1048575) num = 1048575 - num;
    return num;
}

// Converts Hexadecimal to Binary
string HexToBinary (string hex, map <char, string>& binmap){
    string bin = "", bad = "00000000000000000000000000000000";
    for (int i = 0; i < hex.length(); i++){
        hex[i] = tolower(hex[i]);
        if (!(hex[i] <= '9' && hex[i] >= '0') && !(hex[i] <= 'f' && hex[i] >= 'a')){
            bin = bad;
            break;
        }
        bin += binmap[hex[i]];
    }
    return bin;
}

// Initializes the Hexadecimal to Binary Map
void initializeMap (map <char, string>& binmap){
    binmap['1'] = "0001";   binmap['5'] = "0101";   binmap['9'] = "1001";   binmap['d'] = "1101";
    binmap['2'] = "0010";   binmap['6'] = "0110";   binmap['a'] = "1010";   binmap['e'] = "1110";
    binmap['3'] = "0011";   binmap['7'] = "0111";   binmap['b'] = "1011";   binmap['f'] = "1111";
    binmap['4'] = "0100";   binmap['8'] = "1000";   binmap['c'] = "1100";   binmap['0'] = "0000";
}