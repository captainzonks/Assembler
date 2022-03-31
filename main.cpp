#include <iostream>
#include <vector>
#include <fstream>
#include <map>

namespace Assembler {
// 12 bits, 2^12 16 bit locations == 4096
#define MEM_SIZE 4096
    uint memory[MEM_SIZE];

// size of code / data created by assembler
#define CODE_SIZE 4096
    uint machine_code[CODE_SIZE];

    // globals
    uint code_length;   // number of instructions assembled
    uint start_address; // where program begins

    // model for registers
    struct mCPU {
        int AC{};
        uint PC{};
        uint MAR{};
        uint MBR{};
        uint IR{};
        uint INPUT{};
        uint OUTPUT{};
    } mCPU;

    // zero out memory and code segment
    void initialize() {
        for (size_t i{}; i < MEM_SIZE; ++i) {
            memory[i] = 0;
        }
        for (size_t i{}; i < CODE_SIZE; ++i) {
            machine_code[i] = 0;
        }
    }

#define INSTR_LOADX 0x1000
#define INSTR_STOREX 0x2000
#define INSTR_ADD 0x3000
#define INSTR_SUB 0x4000
#define INSTR_INPUT 0x5000
#define INSTR_OUTPUT 0x6000
#define INSTR_HALT 0x7000
#define INSTR_SKIPCOND 0x8000
#define INSTR_JUMPX 0x9000

    /**
     * Simple tokenize function which splits a string of strings
     * on a provided delimiter (e.g., a space), and returns a
     * vector of strings.
     * @param in_string The source string
     * @param delimiter The character to split on
     * @return A vector of separate strings
     */
    std::vector<std::string> tokenize(const std::string &in_string, char delimiter) {
        std::string string;
        std::vector<std::string> vector;

        for (size_t i{}; i < in_string.size(); ++i) {
            if (i != in_string.size() - 1) {
                // if the i-th character is not a delimiter add it to string
                if (in_string.at(i) != delimiter) {
                    string += in_string.at(i);
                }
                    // if delimiter and string is not empty
                    // push string into vector of tokens
                    // make string empty after pushing
                else {
                    if (!string.empty()) {
                        vector.push_back(string);
                        string = "";
                    }
                }
            }
                // if i-th character is last character
                // AND it is not a delimiter
                // add it to string
            else {
                if (in_string.at(i) != delimiter) {
                    string += in_string.at(i);
                }

                // no characters left
                // if string is not empty then push it to vector
                if (!string.empty()) {
                    vector.push_back(string);
                }
            }
        }

        return vector;
    }

    /**
     * Simple 2-pass assembler\n
     * Pass 1: find symbols (lables, variables) and put them in a map\n
     *      (key == symbol_name (string), value == integer value)\n
     * Pass 2: decode the op_code for each instruction (upper 4 bits),
     *  and OR the address operand found with a map lookup in Pass 1 symbol table\n
     * NOTE: No error checking. Everything is case sensitive.
     * @param asm_file_name The assembly file to open
     */
    void assemble(const std::string &asm_file_name) {
        std::fstream new_file;
        std::vector<std::string> asm_lines;

        new_file.open(asm_file_name, std::ios::in); // open file
        if (new_file.is_open()) {
            std::string line;

            // push lines from file into asm_lines vector
            while (getline(new_file, line)) {
                asm_lines.push_back(line);
            }
        }
        new_file.close();

        // pass 1 : find symbol addresses and save them in a map
        //      LABEL, LOAD X
        //          X, DEC 0
        int address{};
        std::map<std::string, int> symbol_table;

        for (const std::string &line: asm_lines) {

            // split on space and get a vector of tokens
            std::vector<std::string> token_one{tokenize(line, ' ')};

            // END is hardcoded as the final instruction
            if (token_one.at(0) == "END") {
                break;
            }

            // is there a comma? we don't want it
            if (token_one.at(0).find(',') != std::string::npos) {

                std::vector<std::string> token_two{tokenize(token_one.at(0), ',')};

                // decrement, hardcoded in uppercase
                if (token_one.at(1) == "DEC") {
                    // first string is the symbol
                    std::string this_symbol{token_two.at(0)};

                    // last string is the value (it's a string, so convert to int)
                    int this_value{std::stoi(token_one.at(2))};

                    // add to symbol table
                    symbol_table.insert(std::pair<std::string, int>(this_symbol, address));

                    // load data value into address location in machine code
                    machine_code[address] = this_value;
                }
                    // first string is the symbol, no value
                else {
                    std::string this_symbol{token_two.at(0)};
                    symbol_table.insert(std::pair<std::string, int>(this_symbol, address));
                }
            }

            // look for jump label and add to symbol table
            // it will be a label and address, but no value
            // TODO: find how this works

            // increment address
            address += 1;
        }

        // pass 2
        // OR OP code with symbol address
        // op_code is upper 4 bits, symbol address is lower 12
        address = 0;    // reset address to 0

        for (const std::string &line: asm_lines) {
            std::vector<std::string> token_one{tokenize(line, ' ')};
            std::string op_code{};
            std::string symbol;

            if (token_one.size() == 1) {
                // instruction is a single op_code
                op_code = token_one.at(0);
            } else {
                if (token_one.size() == 3) {
                    op_code = token_one.at(1);
                    symbol = token_one.at(2);
                } else {
                    op_code = token_one.at(0);
                    symbol = token_one.at(1);
                }
            }

            // remove carriage return character
            if (!symbol.empty() && symbol.at(symbol.size() - 1) == '\r')
                symbol.erase(symbol.size() - 1);            // remove carriage return character
            if (!op_code.empty() && op_code.at(op_code.size() - 1) == '\r')
                op_code.erase(op_code.size() - 1);

            // stop when we get to end
            if (op_code == "END")
                break;

            // symbols are already loaded (value stored at address)
            // these are split to three, look in location 1 for op_code
            if (op_code == "DEC") {
                // do nothing, just increment address, code length
            }

            if (op_code == "LOAD") {
                // takes address operand
                machine_code[address] = INSTR_LOADX;
                machine_code[address] |= symbol_table.at(symbol);
            }

            if (op_code == "STORE") {
                // takes address operand
                machine_code[address] = INSTR_STOREX;
                machine_code[address] |= symbol_table.at(symbol);
            }

            if (op_code == "ADD") {
                // takes address operand
                machine_code[address] = INSTR_ADD;
                machine_code[address] |= symbol_table.at(symbol);
            }

            if (op_code == "SUB") {
                // takes address operand
                machine_code[address] = INSTR_SUB;
                machine_code[address] |= symbol_table.at(symbol);
            }

            if (op_code == "INPUT") {
                // no address operand
                machine_code[address] = INSTR_INPUT;
            }

            if (op_code == "OUTPUT") {
                // no address operand
                machine_code[address] = INSTR_OUTPUT;
            }

            if (op_code == "HALT") {
                // no address operand
                machine_code[address] = INSTR_HALT;
            }

            if (op_code == "SKIPCOND") {
                // TODO: verify if this is right
                // the final operand is a number representing the skip condition
                // SKIPCOND 000 : skip the next instruction if value AC < 0
                // SKIPCOND 400 : skip the next instruction if value AC == 0
                // SKIPCOND 800 : skip the next instruction if value AC > 0

                machine_code[address] = INSTR_SKIPCOND;
                // the skip string is hex to use base 16
                machine_code[address] |= std::stoi(symbol, nullptr, 16);
            }

            if (op_code == "JMP") {
                // takes address operand
                machine_code[address] = INSTR_JUMPX;
                machine_code[address] |= symbol_table.at(symbol);
            }

            address += 1;
            code_length += 1;
        }
    }

    /**
     * Copy the machine code into the memory of the mCPU\n
     * Both memory and machine code are arrays
     */
    void load_code_into_memory() {
        std::cout << "Loading Program: " << code_length << " instructions long." << std::endl;
        for (size_t i{}; i < code_length; ++i) {
            std::cout << "i: " << i << " code: " << std::hex << machine_code[i] << std::endl;
            memory[start_address + i] = machine_code[i];
        }
        mCPU.PC = start_address;
    }

    // INSTRUCTIONS
    // RTL to manipulate registers

    void load_x() {
        std::cout << "loadx -> ";

        mCPU.MBR = memory[mCPU.MAR];            // MBR <- M[MAR]
        mCPU.AC = static_cast<int>(mCPU.MBR);   // AC <- MBR

        std::cout << std::hex << "mCPU.AC == " << mCPU.AC << std::endl;
    }

    void store_x() {
        std::cout << "storex -> ";

        mCPU.MBR = mCPU.AC;             // MBR <- AC
        memory[mCPU.MAR] = mCPU.MBR;    // M[MAR] <- MBR

        std::cout << std::hex << "mem[mCPU.MAR] == " << memory[mCPU.MAR] << std::endl;
    }

    void add_x() {
        std::cout << "addx -> ";

        mCPU.MBR = memory[mCPU.MAR];                      // MBR <- M[MAR]
        mCPU.AC = static_cast<int>(mCPU.AC + mCPU.MBR);   // AC = AC + MBR

        std::cout << std::hex << "mCPU.AC == " << mCPU.AC << std::endl;
    }

    void sub_x() {
        // need to convert to two's complement and add?
        // tricky
        std::cout << "subx -> ";

        mCPU.MBR = memory[mCPU.MAR];                    // MBR <- M[MAR]
        mCPU.AC = static_cast<int>(mCPU.AC - mCPU.MBR); // AC = AC - MBR

        std::cout << std::hex << "mCPU.AC == " << mCPU.AC << std::endl;
    }

    void input() {
        std::cout << "input x" << std::endl;
        mCPU.AC = static_cast<int>(mCPU.INPUT);
    }

    void output() {
        std::cout << "output x" << std::endl;
        mCPU.OUTPUT = mCPU.AC;
    }

    void halt() {
        std::cout << "!HALT!" << std::endl;
    }

    void skipcond() {
        // manipulate PC
        // MAR has IR from decode func

        std::cout << "skipcond" << std::hex << mCPU.MAR << std::endl;
        std::cout << "..... AC: " << mCPU.AC << std::endl;

        // skipcond 000 : skips the next instruction if value AC < 0
        // IR[11-10] == 0
        if (mCPU.MAR == 0x000) {
            if (mCPU.AC < 0)
                mCPU.PC += 1;
        }

            // skipcond 400 : skips the next instruction if value AC == 0
            // IF[11-10] == 01
        else if (mCPU.MAR == 0x400) {
            if (mCPU.AC == 0) {
                mCPU.PC += 1;
                std::cout << "..... INC PC ....." << std::endl;
            }
        }

            // skipcond 800 : skips the next instruction if value AC > 0
            // IR[11-10] == 10
        else if (mCPU.MAR == 0x800) {
            if (mCPU.AC > 0)
                mCPU.PC += 1;
        }
    }

    void jumpx() {
        // MAR contains IR[11-0]
        std::cout << "jumpx" << std::endl;
        mCPU.PC = mCPU.MAR;
    }

    /**
     * Simulates the Fetch -> Decode -> Execute loop
     */
    void fetch_decode_execute() {
        uint op_code{};

        std::cout << "RUNNING: start_address: " << mCPU.PC << std::endl;
        while (true) {
            std::cout << "PC: " << mCPU.PC << std::endl;
            std::cout << "memory: " << std::hex << memory[mCPU.PC] << std::endl;

            // Fetch
            mCPU.MAR = mCPU.PC;             // MAR <- PC
            mCPU.IR = memory[mCPU.MAR];     // IR <- M[MAR]
            mCPU.PC += 1;                   // prepare for next cycle, PC <- PC + 1

            // Decode
            op_code = mCPU.IR & 0xF000;     // decode IR[15-12]
            mCPU.MAR = mCPU.IR & 0x0FFF;    // MAR IR[11-0]

            std::cout << "op_code: " << op_code << " address: " << mCPU.MAR << std::endl;

            // Execute
            switch (op_code) {
                case INSTR_LOADX:
                    load_x();
                    break;
                case INSTR_STOREX:
                    store_x();
                    break;
                case INSTR_HALT:
                    halt();
                    return;
                case INSTR_ADD:
                    add_x();
                    break;
                case INSTR_SUB:
                    sub_x();
                    break;
                case INSTR_INPUT:
                    input();
                    break;
                case INSTR_OUTPUT:
                    output();
                    break;
                case INSTR_SKIPCOND:
                    skipcond();
                    break;
                case INSTR_JUMPX:
                    jumpx();
                    break;
                default:
                    std::cout << "UNKNOWN CMD" << std::endl;
                    return;
            }
        }
    }
}

int main() {

    std::string the_asm_file{"add_two.asm"};

    Assembler::assemble(the_asm_file);
    Assembler::load_code_into_memory();
    Assembler::fetch_decode_execute();

    return 0;
}