#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <bitset>
#include <iterator>

using vec = std::vector<std::string>;
using vec_pair = std::vector<std::pair<int, std::string>>;
using un_mp = std::unordered_map<std::string, int>;
using mp = std::map<int, std::string>;

void leftTrim(std::string& line);
void rightTrim(std::string &line);
void sanitize(std::string& line);
void removeLabel(const std::string& line, std::string& str, std::string ch);
std::string readUntil(std::istream& in, const std::string& delimiters);
void table(vec& lines, vec& hexnum, un_mp& labels, mp& notLab, vec_pair& dupl);
void openFile(int argc, char** argv, vec& lines);
void getHexNums(vec& hexnum, const std::string& byte_str);
void createHexFile(const vec& hexnum, char** argv);
void output(un_mp labels, mp notLab, vec_pair dupl, vec hexnum, vec lines);

// remove leading whitespace
void leftTrim(std::string& line) { 
    auto start = line.find_first_not_of(" \t");
    line = line.substr(start);
}

void rightTrim(std::string& line) {
    // remove characters at ; (comment)
    line = line.substr(0, line.find(";", 0));

    // remove whitespace
    auto end = line.find_last_not_of(" \t");
    line = line.substr(0, end + 1);
}

void sanitize(std::string& line) {
    leftTrim(line);
    rightTrim(line);
}

void removeLabel(const std::string& line, std::string& str, std::string ch) {
    std::string::size_type pos = line.find(ch);
    if (pos != std::string::npos) {
        str = line.substr(0, pos);
    }
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

void getHexNums(vec& hexnum, const std::string& byte_str) {
    std::stringstream str;
    std::bitset<8> x(byte_str);
    unsigned int hex_num;
    std::string result = "";

    str << std::hex << x.to_ulong() << std::endl;
    str >> hex_num;

    for(int i = 1; i >= 0; i--) {
        result += "0123456789ABCDEF"[((hex_num >> i*4) & 0xF)];
    }
    hexnum.emplace_back(result);
}

void table(vec& lines, vec& hexnum, un_mp& labels, mp& notLab, vec_pair& dupl) {
    // opcode
    std::map<std::string, std::string> operands {
        {"r0", "00"}, {"r1", "01"}, {"r2", "10"}, {"r3", "11"}
    };
    std::map<std::string, std::string> instructions {
        {"add", "00"}, {"and", "01"}, {"not", "10"}, {"bnz", "11"}
    };
    
    // iterators
    std::map<std::string, std::string>::iterator it;
    std::map<std::string, int>::iterator it2;
    std::unordered_map<std::string, int>::iterator it3;

    // variables
    std::string y, z, byte_str, str;
    int i = 0;

    // Pass 1: find labels, know address
    for (int i = 0; i < lines.size(); i++) {
        str = lines[i].substr(0,3);
        it = instructions.find(str);
        // not found in instructions map
        if (it == instructions.end()) { 
            // must be a label
            removeLabel(lines[i], str, ":");
            
            it3 = labels.find(str);
            // if this label already exists
            if (it3 != labels.end()) { 
                dupl.emplace_back(std::make_pair(i, str));
            } else {
                labels.insert(std::make_pair(str, i));
            }
            // update lines vector, remove label
            lines[i] = lines[i].substr(lines[i].find(":") + 1);
            sanitize(lines[i]);
        }
    }

    // Get rest of information, Pass 2
    for (auto line: lines) {
        std::stringstream line_to_parse(line);
        std::string word, Rd, Rn, Rm, target;

        while (line_to_parse >> word) {
            byte_str = "";
            sanitize(word);
            it = instructions.find(word);
            // word found in instructions map
            if (it != instructions.end()) { 
                // get instruction op code 
                byte_str += instructions.at(word);
                // get operands op code
                if (word == "not") {
                    // only need Rd, Rn (Rm is xx --> 00)
                    line_to_parse >> Rd;
                    line_to_parse >> Rn;
                    byte_str += operands.at(Rn) + "00";
                    byte_str += operands.at(Rd);
                }
                else if (word == "add" || word == "and") {
                    // need Rd, Rn, and Rm
                    line_to_parse >> Rd;
                    line_to_parse >> Rn;
                    line_to_parse >> Rm;
                    byte_str += operands.at(Rn);
                    byte_str += operands.at(Rm);
                    byte_str += operands.at(Rd);
                } else {
                    // only need target
                    line_to_parse >> target;
                    // get address of label and convert to 6 bits
                    it3 = labels.find(target);
                    if (it3 != labels.end()) { // target found in labels
                        std::bitset<6> y(labels.at(target)); 
                        z = y.to_string(); // convert bits to string
                        byte_str += z;
                    } else { // target label not found
                     notLab.insert(std::make_pair(i, target));
                        byte_str += "000000";
                    }
                }
            }
        }
        getHexNums(hexnum, byte_str);
        i++;
    }
}

void openFile(int argc, char** argv, vec& lines) {
    std::ifstream inFile(argv[1]);
    // std::ifstream inFile("t2.s");
    std::string cur_line;
    bool emptyLine = true;

    // Check if there are enough arguments
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: fiscas <asm file> <hex file> [-l]" << std::endl;
        std::cerr << "-l : print listing to standard error" << std::endl;
        exit(1);
    }

    if (!inFile) { // or !inFile.is_open()
        std::cerr << "Error: Unable to open or read source file '";
        std::cerr << argv[1] << "'" << std::endl;
        exit(2);
    }

    // Parse through file, get labels and address
    while (std::getline(inFile, cur_line)) {
        // check if cur_line is empty
        emptyLine = true;
        for (int i = 0; i < cur_line.length(); i++) {
            if (!isspace(cur_line[i])) {
                emptyLine = false;
                break;
            }
        }

        if (!emptyLine) {
            std::stringstream line_to_parse(cur_line);
            std::string each_line = readUntil(line_to_parse, "\n");
            sanitize(each_line);
            lines.emplace_back(each_line);
        }
    }
    inFile.close();
}

void createHexFile(const std::vector<std::string>& hexnum, char** argv) {
    std::ofstream myFile(argv[2]);
    // std::ofstream myFile("t2.hex");

    myFile << "v2.0 raw\n";
    for (auto c : hexnum) {
        myFile << c << "\n";
    }
    myFile.close();
}

void output(un_mp labels, mp notLab, vec_pair dupl, vec hexnum, vec lines) {
    std::cout << "*** LABEL LIST ***" << std::endl;
    
    // reverse order of map, put it into vector
    std::vector<std::pair<std::string, int>> ordered_labels;

    for (auto label: labels) {
        ordered_labels.push_back(make_pair(label.first, label.second));
    }

    for (int i = ordered_labels.size() - 1; i >= 0; i--) {
        if (ordered_labels[i].second < 10) {
            std::cout << ordered_labels[i].first << "\t:\t0"; 
            std::cout << ordered_labels[i].second << std::endl;
        } else {
            std::cout << ordered_labels[i].first << "\t:\t";
            std::cout << ordered_labels[i].second << std::endl;
        }

        for (int j = dupl.size() - 1; j >= 0; j--) {
            if (dupl[j].second == ordered_labels[i].first) {
                std::cerr << "Error: Label " << ordered_labels[i].first;
                std::cerr << " is already defined." << std::endl;
            }
        }
    }

    std::cout << "*** MACHINE PROGRAM ***" << std::endl;
    for (int i = 0; i < lines.size(); i++) {
        std::string result = "";
        std::string temp = std::to_string(i + 1);

        if (hexnum[i] == "C0") {
            std::cerr << "Error: Label " << notLab.at(i) << " at line ";
            std::cerr << i + 1 << " is not in the symbol table." << std::endl;
        }
        if (i + 1 < 10) {
            result += "0" + temp + ":" + hexnum[i] + "\t" + lines[i] + "\n";
        } else {
            result += temp + ":" + hexnum[i] + "\t" + lines[i] + "\n";
        }
        std::cout << result;
    }
}

int main(int argc, char** argv) {
    // Pass 1: Build your table of symbols and their values 
    // Pass 2: Create your output using the data stored the symbol table
    // store information
    std::vector<std::string> lines;
    std::vector<std::string> hexnum;
    std::unordered_map<std::string, int> labels;
    std::map<int, std::string> notLab;
    std::vector<std::pair<int, std::string>> dupl;

    openFile(argc, argv, lines);
    table(lines, hexnum, labels, notLab, dupl);

    // Output in .hex file
    createHexFile(hexnum, argv);

    if (argc == 4) {
        std::string comment = argv[3];
        if (comment == "-l") {
            output(labels, notLab, dupl, hexnum, lines);
        } else {
            std::cerr << "Usage: fiscas <asm file> <hex file> [-l]\n";
            std::cerr << "-l : print listing to standard error\n";
        }
    } 
    return 0;
}