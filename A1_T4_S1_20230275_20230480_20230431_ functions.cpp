#include "A1_T4_S1_20230275_20230480_20230431.h"
#include <cmath>


// Helper to get the exponent from the bits
int extractExponent(int operand) {
    int exponentBits = (operand & 0b01111000) >> 3;
    return exponentBits - 4;  // Applying bias of 4
}

// Helper to get the mantissa from the bits
float extractMantissa(int operand) {
    int mantissaBits = operand & 0b00000111;
    return 1.0f + (mantissaBits / 8.0f);  // Dividing by 8 for normalization
}

float operandToFloat(int operand) {
    // Extract the sign
    int signBit = (operand & 0b10000000) >> 7;
    float sign = (signBit == 0) ? 1.0f : -1.0f;

    // Extract exponent and apply bias
    int exponent = extractExponent(operand);

    // Extract mantissa and normalize
    float mantissa = extractMantissa(operand);

    // Calculate final float value
    return sign * mantissa * std::pow(2.0f, exponent);
}

int floatToOperand(float value) {
    // Determine sign
    int signBit = (value < 0) ? 1 : 0;

    // Convert to absolute value
    value = std::abs(value);

    // Calculate exponent
    int exponent = static_cast<int>(std::log2(value));
    int exponentBits = exponent + 4;  // Adding bias of 4

    // Calculate mantissa by removing exponent contribution
    float mantissaValue = value / std::pow(2.0f, exponent);
    int mantissaBits = static_cast<int>((mantissaValue - 1.0f) * 8);

    // Combine bits: sign, exponent, and mantissa
    return (signBit << 7) | (exponentBits << 3) | mantissaBits;
}

Register::Register() : registers() {}

int Register::getCell(int address) const
{
    return (address >= 0 && address < size) ? registers[address] : 0;
}

void Register::setCell(int address, int value)
{
    if (address >= 0 && address < size) {
        registers[address] = value;
    }
}

// Memory Class
Memory::Memory() : memory() {}

int Memory::getCell(int address) const
{
    return (address >= 0 && address < size) ? memory[address] : 0;
}

void Memory::setCell(int address, int value)
{
    if (address >= 0 && address < size) {
        memory[address] = value;
    }
}

// CU Class
void CU::load1(int regAddr, int memAddr, Register &reg, Memory &mem)
{
    reg.setCell(regAddr, mem.getCell(memAddr));
}

void CU::load2(int regAddr, int value, Register &reg, Memory &mem)
{
    reg.setCell(regAddr, value);
}

void CU::store(int regAddr, int memAddr, Register &reg, Memory &mem)
{
    mem.setCell(memAddr, reg.getCell(regAddr));
}

void CU::move(int srcReg, int destReg, Register &reg)
{
    reg.setCell(destReg, reg.getCell(srcReg));
}

void CU::jump(int regAddr, int &programCounter, Register &reg)
{
    programCounter = reg.getCell(regAddr);
}

void CU::bitwiseOr(int r, int s, int t, Register &reg) {
    reg.setCell(r, reg.getCell(s) | reg.getCell(t));
}

void CU::bitwiseAnd(int r, int s, int t, Register &reg) {
    reg.setCell(r, reg.getCell(s) & reg.getCell(t));
}

void CU::bitwiseXor(int r, int s, int t, Register &reg) {
    reg.setCell(r, reg.getCell(s) ^ reg.getCell(t));
}

void CU::rotateRight(int r, int x, Register &reg) {
    // Get the value from the register
    int value = reg.getCell(r);

    // Convert to 16-bit binary representation by masking it to fit 16 bits
    int mask = 0xFFFF;
    value &= mask;  // Ensure we're working with a 16-bit value

    // Perform the circular right rotation
    int rotatedValue = ((value >> x) | (value << (16 - x))) & mask;

    // Store the rotated value back in the register
    reg.setCell(r, rotatedValue);
}


void CU::conditionalJump(int r, int xy, int &programCounter, Register &reg) {
    // Retrieve the value in register `r` and register `0`
    int valueR = reg.getCell(r);
    int value0 = reg.getCell(0);

    // Interpret values as two's complement (16-bit signed integers)
    if (valueR & 0x8000) {  // Check if the highest bit (sign bit) is set for a 16-bit value
        valueR -= 0x10000;   // Convert `valueR` to its negative form if sign bit is set
    }
    if (value0 & 0x8000) {
        value0 -= 0x10000;   // Convert `value0` to its negative form if sign bit is set
    }

    // Perform the conditional jump
    if (valueR > value0) {
        programCounter = xy;
    }
}


void  CU::halt()
{
    cout << "Program execution halted.\n";
   haltt=!haltt;
}

// CPU Class
void CPU::fetch(Memory &mem)
{
    if (programCounter < Memory::size) {
        try {
            int instruction = mem.getCell(programCounter) << 8 |
                              mem.getCell(programCounter + 1);
            std::cout << programCounter << " " << std::hex << instruction
                      << "\n";
            programCounter += 2;
            execute(instruction, mem);
        } catch (const invalid_argument &e) {
            cerr << "Error: Invalid instruction encountered at address "
                 << programCounter - 1 << ": "
                 << mem.getCell(programCounter - 1) << endl;
            exit(1); // Halt execution on invalid instruction
        } catch (const out_of_range &e) {
            cerr << "Error: Instruction out of range at address "
                 << programCounter - 1 << endl;
            exit(1);
        }
    } else {
        cout << "Program Counter out of bounds.\n";
        exit(1);
    }
}

void CPU::execute(int instruction, Memory &mem)
{
    int opcode = (instruction >> 12) & 0xF;
    int r = (instruction >> 8) & 0xF;
    int s = (instruction >> 4) & 0xF;
    int t = instruction & 0xF;

    try {
        switch (opcode) {
            case 1:
                cu.load1(r, (s << 4) | t, reg, mem);
                break;
            case 2:
                cu.load2(r, (s << 4) | t, reg, mem);
                break;
            case 3:
                cu.store(r, (s << 4) | t, reg, mem);
                break;
            case 4:
                CU::move(r, s, reg);
                break;
            case 5: {
                reg.setCell(r, reg.getCell(s) + reg.getCell(t));
                break;
            }
            case 6: {
                reg.setCell(r, floatToOperand(operandToFloat(reg.getCell(s)) + operandToFloat(t)));
            }
            case 7:
                cu.bitwiseOr(r, s, t, reg);
                break;
            case 8:
                cu.bitwiseAnd(r, s, t, reg);
                break;
            case 9:
                cu.bitwiseXor(r, s, t, reg);
                break;
            case 0xB:
                cu.jump(r, programCounter, reg);
                break;
            case 0xA:
                cu.rotateRight(r, s, reg);
                break;
            case 0xD:
                cu.conditionalJump(r, (s << 4) | t, programCounter, reg);
                break;
            case 0xC:
                cu.halt();
                break;
            default:
                cu.halt();
                break;
        }
    } catch (const runtime_error &e) {
        cerr << "Error during execution: " << e.what() << endl;
        exit(1);
    }
}

// Machine Class

void Machine::loadProgram(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }
    int instruction, i = 0;
    while (i < Memory::size && file >> hex >> instruction) {
        memory.setCell(i++, (instruction >> 8) & 0xFF);
        memory.setCell(i++, (instruction & 0xFF));
    }
    file.close();
}

void Machine::run()
{
    while (!haltt) {
        fetch(memory);
    }
}

void Machine::outputState() const
{
    cout << "Registers:\n";
    for (int i = 0; i < 16; ++i) {
        cout << "R" << i << ": " << reg.getCell(i) << "\n";
    }
    cout << "Memory:\n";
    for (int i = 0; i < 256; ++i) {
        cout << setw(2) << i << ": " << memory.getCell(i) << "\n";
    }
}

// MainUI Class
void MainUI::displayMenu()
{
    while (true) {
        cout << "1. Load Program\n2. Run Program\n3. Display State\n4. "
                "Exit\n";
        char choice;
        cin >> choice;

        switch (choice) {
            case '1': {
                string filename;
                cout << "Enter filename: ";
                cin >> filename;
                machine.loadProgram(filename);
                break;
            }
            case '2':
                machine.run();
                break;
            case '3':
                machine.outputState();
                break;
            case '4':
                cout << "thanks for using our program!\n";
                return;
            default:
                cout << "Invalid choice. Please select an option from the "
                        "menu.\n";
                break;
        }
    }
}
