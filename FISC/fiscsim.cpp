#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <bitset>
#include <locale>

using vec = std::vector<std::string>;
using mp = std::map<std::string, std::string>;

bool isEmptyLine(std::string cur_line);
std::string readUntil(std::istream& in, const std::string& del);
void openFile(int argc, char** argv, vec& lines);
void binaryToHex(std::string& byte_str);
std::string hexToBinary(const std::string& hex_num);
std::string convertToHex(const unsigned int& Rd);
void convertToNum(std::string& target);
void notFunc(const std::string& rd, const std::string& rn, mp& registers);
void addFunc(std::string rd, std::string rn, std::string rm, mp& registers);
void andFunc(std::string rd, std::string rn, std::string rm, mp& registers);
void decodeHex(vec lines, int totalCycles, int argc, std::string option);

bool isEmptyLine(std::string cur_line) {
    for (int i = 0; i < cur_line.length(); i++) {
        if (!isspace(cur_line[i])) {
            return false;
        }
    }
    return true;
}

std::string readUntil(std::istream& in, const std::string& del) {
    // delimiter --> \n
    std::string result;
    char ch;
    while (in.get(ch)) {
        if (std::find(del.begin(), del.end(), ch) != del.end()) {
            break;
        }
        result.push_back(ch);
    }
    return result;
}

void openFile(int argc, char** argv, vec& lines) {
    std::ifstream inFile(argv[1]);
    // std::ifstream inFile("t1.hex");
    std::string cur_line;
    bool emptyLine = true;

    // Check if there are enough arguments
    if (argc < 2 || argc > 4) {
        std::cerr << "Usage: fiscsim <object file> [cycles] [-d]\n";
        std::cerr << "-d : print disassembly listing with each cycle\n";
        std::cerr << "if cycles are unspecified the CPU will run";
        std::cerr << " for 20 cycles" << std::endl;
        exit(1);
    }

    if (!inFile) { // or !inFile.is_open()
        std::cerr << "Error: Unable to open or read hex file '";
        std::cerr << argv[1] << "'" << std::endl;
        exit(2);
    }

    // Parse through file, get labels and address
    std::getline(inFile, cur_line); // get v2.0 raw, skip first line
    while (std::getline(inFile, cur_line)) {
        // check if cur_line is empty
        emptyLine = isEmptyLine(cur_line);
        if (!emptyLine) {
            std::stringstream line_to_parse(cur_line);
            std::string each_line = readUntil(line_to_parse, "\n");
            lines.emplace_back(each_line);
        }
    }
    inFile.close();
}

void binaryToHex(std::string& byte_str) {
    std::stringstream str;
    std::bitset<8> x(byte_str);
    unsigned int hex_num;
    std::string result = "";

    str << std::hex << x.to_ulong() << std::endl;
    str >> hex_num;

    for(int i = 1; i >= 0; i--) {
        result += "0123456789ABCDEF"[((hex_num >> i*4) & 0xF)];
    }
    byte_str = result;
}

std::string hexToBinary(const std::string& hex_num) {
    std::stringstream ss;
    unsigned int binary_num;
    std::string result = "";

    ss << std::hex << hex_num;
    ss >> binary_num;

    for (int i = 7; i >= 0; i--) {
        // used bitwise operator to shift right
        // bitwise operator & used to compare
        result += ((binary_num >> i) & 1) ? "1" : "0";
    }
    return result;
}

std::string convertToHex(const unsigned int& Rd) {
    // convert number to hex number
    std::string result = "";
    for (int i = 1; i >= 0; i--) {
        result += "0123456789ABCDEF"[((Rd >> i*4) & 0xF)];
    }
    return result;
}

void convertToNum(std::string& target) {
    // target will be hex number --> convert to decimal
    std::stringstream ss;
    unsigned int num;
    ss << std::hex << target;
    ss >> num;
    target = std::to_string(num);
}

void notFunc(const std::string& rd, const std::string& rn, mp& registers) {
    std::stringstream ss;
    unsigned int Rd, Rn;
    
    // convert hex numbers of registers to binary
    // and use not bitwise operator
    ss << std::hex << registers.at(rn);
    ss >> Rn;
    Rd = ~Rn;   // 1's complement
    // convert to hex numbers and update registers container
    registers[rd] = convertToHex(Rd);
}

void addFunc(std::string rd, std::string rn, std::string rm, mp& registers) {
    std::stringstream ss1, ss2;
    unsigned int Rd, Rn, Rm, carry;
    std::string result = "";
    
    // convert hex numbers of registers to binary
    ss1 << std::hex << registers.at(rn);
    ss1 >> Rn;
    ss2 << std::hex << registers.at(rm);
    ss2 >> Rm;

    // add rn and rm using bitwise operators
    while (Rm != 0) {
        carry = Rn & Rm; // carry value
        Rn = Rn ^ Rm; // sum value
        Rm = carry << 1; // carrry is shifted to the left
    }
    Rd = Rn;
    // convert to hex numbers and update registers container
    registers[rd] = convertToHex(Rd);
}

void andFunc(std::string rd, std::string rn, std::string rm, mp& registers) {
    std::stringstream ss1, ss2;
    unsigned int Rd, Rn, Rm, carry;
    std::string result = "";

    // convert hex numbers of registers to binary
    ss1 << std::hex << registers.at(rn);
    ss1 >> Rn;
    ss2 << std::hex << registers.at(rm);
    ss2 >> Rm;

    // Rd <- rn * rm using bitwise operator AND
    Rd = Rn & Rm;
    // convert to hex numbers and update registers container
    registers[rd] = convertToHex(Rd);
}

void decodeHex(vec lines, int totalCycles, int argc, std::string option) {
    // opcode
    std::map<std::string, std::string> operands {
        { "00", "r0"}, {"01", "r1"}, {"10", "r2"}, {"11", "r3"}
    };
    std::map<std::string, std::string> instructions {
        {"00", "add"}, {"01", "and"}, {"10", "not"}, {"11", "bnz"}
    };

    // keep track of registers
    std::map<std::string, std::string> registers {
         {"r0", "00"}, {"r1", "00"}, {"r2", "00"}, {"r3", "00"}
    };

    std::vector<std::string> disassemblyVec;
    std::string instruction, rd, rn, rm, target;
    std::string byte_str, op_code;
    std::string disassembly;
    std::string hex_num;
    int pc = 0, z = 0;

    // cycle
    for (int cycle = 1; cycle < totalCycles + 1; cycle++) {
        // convert hex number to binary number
        byte_str = hexToBinary(lines[pc]);
        disassembly = "";

        // decode binary number
        for (int j = 0; j < byte_str.length() - 1; j = j + 2) {
            op_code = "";
            hex_num = "";
            op_code += byte_str[j];
            op_code += byte_str[j+1];
            if (j == 0) {
                instruction = instructions.at(op_code); 
                if (instruction == "bnz") {
                    // get 6 bits after first 2 bits
                    target = byte_str.substr(2, byte_str.length() - 1);
                    // need to convert target to hex num
                    hex_num += "00" + target;
                    binaryToHex(hex_num);
                    target = hex_num; // update target
                    break;
                }
            } else if (j == 2) {
                rn = operands.at(op_code);
            } else if (j == 4) {
                rm = operands.at(op_code);
            } else {
                rd = operands.at(op_code);
            }
        }
        // getting line of code
        disassembly += instruction + " ";

        if (instruction == "add" || instruction == "and") {
            disassembly += rd + " ";
            disassembly += rn + " ";
            disassembly += rm;

            // add, rd <- rn + rm
            if (instruction == "add") {
                addFunc(rd, rn, rm, registers);
            } else {
                // and, rd <- rn * rm
                andFunc(rd, rn, rm, registers);
            }
        }
        else if (instruction == "not") {
            disassembly += rd + " ";
            disassembly += rn;
            // rd <- not rn
            notFunc(rd, rn, registers);
        } else {
            convertToNum(target);
            disassembly += target;
        }    
        disassemblyVec.emplace_back(disassembly);

        // convert Rd to an int
        std::stringstream ss;
        unsigned int Rd;
        ss << std::hex << registers.at(rd);
        ss >> Rd;

        // update z flag
        if (Rd == 0) {
            z = 1;
        } else {
            z = 0;
        }

        // update pc
        if (instruction == "bnz") {
            if (!z) {
                pc = stoi(target); // convert str to int
            } else {
                pc = pc + 1;
            }
        } else {
            pc = pc + 1;
        }

        // --------------- output ---------------
        std::cout << "Cycle:" << std::to_string(cycle) << " ";
        if (pc < 10) {
            std::cout << "State:PC:0" << std::to_string(pc) << " ";
        } else {
            std::cout << "State:PC:" << std::to_string(pc) << " ";
        }
        std::cout << "Z:" << std::to_string(z) << " ";
        std::cout << "R0: " << registers.at("r0") << " ";
        std::cout << "R1: " << registers.at("r1") << " ";
        std::cout << "R2: " << registers.at("r2") << " ";
        std::cout << "R3: " << registers.at("r3") << std::endl;
        // if there is -d, then print out disassembly
        if ((argc >= 3) && (option == "-d")) {
            std::cout << "Disassembly: " << disassembly;
            std::cout << std::endl << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    std::vector<std::string> lines;
    std::string option = "";
    std::locale loc;
    int totalCycles = 20;   // default

    // if only three or 4 command line arguments
    if (argc >= 3) {
        std::string temp(argv[2]);
        if (argc == 3) {
            // argv[2] can either be a number or -d
            if(!(isdigit(temp[0], loc))) {
                // argv[2] is -d
                option = temp;
            } else {
                // convert string to int
                totalCycles = stoi(temp);
            }
        } else if (argc == 4) {
            totalCycles = atoi(argv[2]);
            option = argv[3];
        }
    }

    openFile(argc, argv, lines);
    decodeHex(lines, totalCycles, argc, option);

    return 0;
}