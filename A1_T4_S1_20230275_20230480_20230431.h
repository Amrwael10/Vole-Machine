#pragma once
#include "A1_T4_S1_20230275_20230480_20230431.h"
#include <bits/stdc++.h>
using namespace std;

static int haltt = 0;
static int programCounter =0;

class Register
{
protected:
    static const int size = 16;
    int registers[size];

public:
    Register();
    int getCell(int address) const;
    void setCell(int address, int value);
};

// Memory Class
class Memory
{
public:
    static const int size = 256;
    int memory[size];

    Memory();

    int getCell(int address) const;
    void setCell(int address, int value);
};

// CU Class
class CU
{
public:
    static void load1(int regAddr, int memAddr, Register &reg, Memory &mem);
    static void load2(int regAddr, int value, Register &reg, Memory &mem);
    static void store(int regAddr, int memAddr, Register &reg, Memory &mem);
    static void move(int srcReg, int destReg, Register &reg);
    static void jump(int regAddr, int &programCounter, Register &reg);
    static void bitwiseOr(int r, int s, int t, Register &reg);
    static void bitwiseAnd(int r, int s, int t, Register &reg);
    static void bitwiseXor(int r, int s, int t, Register &reg);
    static void rotateRight(int r, int x, Register &reg);
    static void conditionalJump(int r, int xy, int &programCounter, Register &reg);
    static void  halt();
};

// CPU Class
class CPU{
protected:
    Register reg;
    CU cu;

public:
    void fetch(Memory &mem);
    void execute(int instruction, Memory &mem);
};
// Initialize the static member

// Machine Class
class Machine : public CPU
{
private:
    Memory memory;

public:
    void loadProgram(const string &filename);

    void run();

    void outputState() const;
};

// MainUI Class
class MainUI
{
    Machine machine;

public:
    void displayMenu();
};