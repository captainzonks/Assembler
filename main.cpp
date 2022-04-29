#include <iostream>
#include <vector>
#include <fstream>
#include <map>

namespace Assembler {
// 12 bits, 2^12 16 bit locations == 4096
#define MEM_SIZE 4096
    uint16_t memory[MEM_SIZE];

// size of code / data created by assembler
#define CODE_SIZE 4096
    uint16_t machine_code[CODE_SIZE];

    // globals
    uint16_t code_length;   // number of instructions assembled
    uint16_t start_address; // where program begins

    // model for registers
    struct mCPU {
        int AC{};
        uint16_t SP{2000};      // new register, stack pointer points at 2000 in the main memory
        uint16_t PC{};
        uint16_t MAR{};
        uint16_t MBR{};
        uint16_t IR{};
        uint16_t INPUT{};
        uint16_t OUTPUT{};
    } mCPU;

    // zero out memory and code segment and stack
    void initialize() {
        for (size_t i{}; i < MEM_SIZE; ++i) {
            memory[i] = 0;
        }
        for (size_t i{}; i < CODE_SIZE; ++i) {
            machine_code[i] = 0;
        }
//        for (size_t i{}; i < STACK; ++i) {
//            stack[i] = 0;
//        }
    }

    // Basic
#define INSTR_LOADX 0x1000
#define INSTR_STOREX 0x2000
#define INSTR_ADD 0x3000
#define INSTR_SUB 0x4000
#define INSTR_INPUT 0x5000
#define INSTR_OUTPUT 0x6000
#define INSTR_HALT 0x7000
#define INSTR_SKIPCOND 0x8000
#define INSTR_JUMPX 0x9000

    // Extended
//#define INSTR_CLEAR 0xA000
#define INSTR_LOADI 0xB000
#define INSTR_STOREI 0xD000

    // SP extended architecture
#define INSTR_PUSH 0xE000
#define INSTR_POP 0xF000
#define INSTR_CALL 0xA000
#define INSTR_RET 0xC000

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

                if (token_one.empty()) continue; // to prevent out of range error

                // decimal, hardcoded in uppercase
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
//                if (token_one.at(1) == "HEX")
//                {
//                    // first string is the symbol
//                    std::string this_symbol{token_two.at(0)};
//
//                    // last string is the value (it's a string, so convert to int)
//                    int this_value{std::stoi(token_one.at(2))};
//
//                    // add to symbol table
//                    symbol_table.insert(std::pair<std::string, int>(this_symbol, address));
//
//                    // load data value into address location
//                    machine_code[address] = this_value;
//                }
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

            if (token_one.empty()) continue; // to prevent out of range error

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
            else if (op_code == "DEC") {
                // do nothing, just increment address, code length
            } else if (op_code == "PROC") {
                // do nothing
            } else if (op_code == "ENDP") {
                // do nothing
            } else if (op_code == "LOAD") {
                // takes address operand
                machine_code[address] = INSTR_LOADX;
                machine_code[address] |= symbol_table.at(symbol);
            } else if (op_code == "STORE") {
                // takes address operand
                machine_code[address] = INSTR_STOREX;
                machine_code[address] |= symbol_table.at(symbol);
            } else if (op_code == "ADD") {
                // takes address operand
                machine_code[address] = INSTR_ADD;
                machine_code[address] |= symbol_table.at(symbol);
            } else if (op_code == "SUB") {
                // takes address operand
                machine_code[address] = INSTR_SUB;
                machine_code[address] |= symbol_table.at(symbol);
            } else if (op_code == "INPUT") {
                // no address operand
                machine_code[address] = INSTR_INPUT;
            } else if (op_code == "OUTPUT") {
                // no address operand
                machine_code[address] = INSTR_OUTPUT;
            } else if (op_code == "HALT") {
                // no address operand
                machine_code[address] = INSTR_HALT;
            } else if (op_code == "SKIPCOND") {
                // TODO: verify if this is right
                // the final operand is a number representing the skip condition
                // SKIPCOND 000 : skip the next instruction if value AC < 0
                // SKIPCOND 400 : skip the next instruction if value AC == 0
                // SKIPCOND 800 : skip the next instruction if value AC > 0

                machine_code[address] = INSTR_SKIPCOND;
                // the skip string is hex to use base 16
                machine_code[address] |= std::stoi(symbol, nullptr, 16);
            } else if (op_code == "JMP") {
                // takes address operand
                machine_code[address] = INSTR_JUMPX;
                machine_code[address] |= symbol_table.at(symbol);
            } else if (op_code == "CLEAR") {
                // no address operand
//                machine_code[address] = INSTR_CLEAR;
            }
            else if (op_code == "RET") {
                // no address operand
                machine_code[address] = INSTR_RET;
            } else if (op_code == "CALL") {
                // jump to subroutine
                // takes address operand
                machine_code[address] = INSTR_CALL;
                machine_code[address] |= symbol_table.at(symbol);
            } else if (op_code == "LOADI") {
                // load indirect
                // takes pointer operand
                machine_code[address] = INSTR_LOADI;
                machine_code[address] |= symbol_table.at(symbol);
            } else if (op_code == "STOREI") {
                // store indirect
                // takes pointer operand
                machine_code[address] = INSTR_STOREI;
                machine_code[address] |= symbol_table.at(symbol);
            } else if (op_code == "PUSH") {
                // push AC value to stack
                machine_code[address] = INSTR_PUSH;
            } else if (op_code == "POP") {
                // pop from stack
                // takes address operand
                machine_code[address] = INSTR_POP;
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
//        std::cout << "LOAD X -> ";

        mCPU.MBR = memory[mCPU.MAR];            // MBR <- M[MAR]
        mCPU.AC = static_cast<int>(mCPU.MBR);   // AC <- MBR

//        std::cout << std::hex << "mCPU.AC == " << mCPU.AC << std::endl;
    }

    void store_x() {
//        std::cout << "STORE X -> ";

        mCPU.MBR = mCPU.AC;             // MBR <- AC
        memory[mCPU.MAR] = mCPU.MBR;    // M[MAR] <- MBR

//        std::cout << std::hex << "mem[mCPU.MAR] == " << memory[mCPU.MAR] << std::endl;
    }

    void add_x() {
//        std::cout << "ADD X -> ";

        mCPU.MBR = memory[mCPU.MAR];                      // MBR <- M[MAR]
        mCPU.AC = static_cast<int>(mCPU.AC + mCPU.MBR);   // AC = AC + MBR

//        std::cout << std::hex << "mCPU.AC == " << mCPU.AC << std::endl;
    }

    void sub_x() {
        // need to convert to two's complement and add?
        // tricky
//        std::cout << "SUB X -> ";

        mCPU.MBR = memory[mCPU.MAR];                    // MBR <- M[MAR]
        mCPU.AC = static_cast<int>(mCPU.AC - mCPU.MBR); // AC = AC - MBR

//        std::cout << std::hex << "mCPU.AC == " << mCPU.AC << std::endl;
    }

    void input() {
//        std::cout << "INPUT X: ";
        std::cin >> mCPU.INPUT;
        mCPU.AC = static_cast<int>(mCPU.INPUT);
    }

    void output() {
        std::cout << "OUTPUT X == ";
        mCPU.OUTPUT = mCPU.AC;
        std::cout << "Value: " << std::dec << (char)mCPU.OUTPUT << std::endl;
    }

    void halt() {
        std::cout << "!HALT!" << std::endl;
    }

    void skipcond() {
        // manipulate PC
        // MAR has IR from decode func

//        std::cout << "SKIPCOND" << std::hex << mCPU.MAR << std::endl;
//        std::cout << "..... AC: " << mCPU.AC << std::endl;

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
//                std::cout << "..... INC PC ....." << std::endl;
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
//        std::cout << "JUMP X" << std::endl;
        mCPU.PC = mCPU.MAR;
    }

    void clear() {
//        std::cout << "CLEAR" << std::endl;
        mCPU.AC = 0; // AC <- 0
    }

    void ret() {
        // MAR contains IR[0-11]
        // Assumes return address is at top of stack

        /******* POP return address off stack ***********/
        mCPU.MBR = memory[mCPU.SP - 1];          // MBR <- stack top value
        // set the PC to the value retrieved from top of stack
        mCPU.PC = mCPU.MBR;                     // M[MAR] <- PC
        // move the SP value to the buffer
        mCPU.MBR = mCPU.SP;
        // move the value of SP from the buffer to the accumulator
        mCPU.AC = mCPU.MBR;
        // decrement the value of AC
        mCPU.AC -= 1;
        // move from the AC to the buffer
        mCPU.MBR = mCPU.AC;
        // move from the buffer to the SP
        mCPU.SP = mCPU.MBR;
        /***********************************************/

//        std::cout << "RETURN : mCPU.PC == " << mCPU.PC << std::endl;
    }

    void call() {
        // MAR contains IR[0-11]
        //      call to subroutine
        //      specified in the address field

        // Save PC (return address) in memory location
        // at the subroutine address

        mCPU.MBR = mCPU.PC;

        // MAR has IR[0-11] already

        // move PC to AC to push it
        mCPU.AC = mCPU.MBR;

        /***** PUSH return address to stack */
        // store the value in the AC on the stack
        memory[mCPU.SP] = mCPU.AC;
        // load the stack pointer into the buffer
        mCPU.MBR = mCPU.SP;
        // load the stack pointer to the accumulator
        mCPU.AC = mCPU.MBR;
        // increment by a hardwired 1
        mCPU.AC += 1;
        // move value of AC back to buffer register
        mCPU.MBR = mCPU.AC;
        // move from buffer to the SP
        mCPU.SP = mCPU.MBR;
        /************************************/

        // now move PC to the subroutine address (plus 1)
        mCPU.MBR = mCPU.MAR;
        // assumes hardware can do this! Needs a 1
        // as a hardwired add option
        mCPU.AC = 1;
        mCPU.AC = mCPU.AC + mCPU.MBR;
        mCPU.PC = mCPU.AC;
        // could also do:
        // mCPU.AC = mCPU.AC + mCPU.MBR; (still need to increment by 1)
        // mCPU.PC = mCPU.AC;
        // mCPU.PC = mCPU.PC + 1 (using same PC increment hardware capability in Fetch in main loop)
//        std::cout << std::hex << "CALL : mCPU.PC == " << mCPU.PC << std::endl;
    }

    void loadi() {
        // MAR contains IR[0-11]
        // load the address stored within pointer variable
        mCPU.MBR = memory[mCPU.MAR];
        // move the address from buffer to address register
        mCPU.MAR = mCPU.MBR;
        // load the value stored at the address provided by the pointer
        mCPU.MBR = memory[mCPU.MAR];
        // move value from buffer to accumulator
        mCPU.AC = mCPU.MBR;
//        std::cout << std::hex << "mCPU.AC == " << mCPU.AC << std::endl;
    }

    void storei() {
        // MAR contains IR[0-11]
        // load the address stored within pointer variable
        mCPU.MBR = memory[mCPU.MAR];
        // move the address from buffer to address register
        mCPU.MAR = mCPU.MBR;
        // move accumulator value into buffer
        mCPU.MBR = mCPU.AC;
        // move buffer value into memory at provided address
        memory[mCPU.MAR] = mCPU.MBR;
//        std::cout << std::hex << "memory[mCPU.MAR] == " << memory[mCPU.MAR] << std::endl;
    }

    void push() {
        // store the value in the AC on the stack
        memory[mCPU.SP] = mCPU.AC;
        // load the stack pointer into the buffer
        mCPU.MBR = mCPU.SP;
        // load the stack pointer to the accumulator
        mCPU.AC = mCPU.MBR;
        // increment by a hardwired 1
        mCPU.AC += 1;
        // move value of AC back to buffer register
        mCPU.MBR = mCPU.AC;
        // move from buffer to the SP
        mCPU.SP = mCPU.MBR;
//        std::cout << std::hex << "Value pushed to stack == " << memory[mCPU.SP - 1] << std::endl;
    }

    void pop() {
        // MAR contains IR[0-11]

        mCPU.MBR = memory[mCPU.SP - 1];        // MBR <- stack top value
        memory[mCPU.MAR] = mCPU.MBR;          // M[MAR] <- MBR
        // move the SP value to the buffer
        mCPU.MBR = mCPU.SP;
        // move the value of SP from the buffer to the accumulator
        mCPU.AC = mCPU.MBR;
        // decrement the value of AC
        mCPU.AC -= 1;
        // move from the AC to the buffer
        mCPU.MBR = mCPU.AC;
        // move from the buffer to the SP
        mCPU.SP = mCPU.MBR;
//        std::cout << std::hex << "Value popped from stack == " << memory[mCPU.SP] << std::endl;
    }

    /**
     * Simulates the Fetch -> Decode -> Execute loop
     */
    void fetch_decode_execute() {
        uint16_t op_code{};

//        std::cout << "RUNNING: start_address: " << mCPU.PC << std::endl;
        while (true) {
//            std::cout << std::endl;
//            std::cout << "PC: " << mCPU.PC << std::endl;
//            std::cout << "memory: " << std::hex << memory[mCPU.PC] << std::endl;

            // Fetch
            mCPU.MAR = mCPU.PC;             // MAR <- PC
            mCPU.IR = memory[mCPU.MAR];     // IR <- M[MAR]
            mCPU.PC += 1;                   // prepare for next cycle, PC <- PC + 1

            // Decode
            op_code = mCPU.IR & 0xF000;     // decode IR[15-12]
            mCPU.MAR = mCPU.IR & 0x0FFF;    // MAR IR[11-0]

//            std::cout << "op_code: " << op_code << " address: " << mCPU.MAR << std::endl;

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
//                case INSTR_CLEAR:
//                    clear();
//                    break;
                case INSTR_RET:
                    ret();
                    break;
                case INSTR_CALL:
                    call();
                    break;
                case INSTR_LOADI:
                    loadi();
                    break;
                case INSTR_STOREI:
                    storei();
                    break;
                case INSTR_PUSH:
                    push();
                    break;
                case INSTR_POP:
                    pop();
                    break;
                default:
                    std::cout << "UNKNOWN CMD" << std::endl;
                    return;
            }
        }
    }
}

int main() {

//    std::string the_asm_file{"add_two.asm"};
//    std::string the_asm_file{"subt_two.asm"};
//    std::string the_asm_file{ "loop_add.asm" };
//    std::string the_asm_file{"jump.asm"};
//    std::string the_asm_file{"stack.asm"};
    std::string the_asm_file{"string.asm"};

    Assembler::initialize();
    Assembler::assemble(the_asm_file);
    Assembler::load_code_into_memory();
    Assembler::fetch_decode_execute();

    return 0;
}
