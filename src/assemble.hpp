#pragma once

#include <vector>
#include <memory>
#include <string>
#include "Transition.h"

class AssemblyInstruction {

protected:
    const int opCode;
    AssemblyInstruction(int opCode) : opCode(opCode) {}
public:
    virtual std::string getAssembly() = 0;
    virtual ~AssemblyInstruction() = default;
    [[nodiscard]] int getOpCode() const {
        return opCode;
    }
};

//other instructions classes here



std::vector<std::unique_ptr<AssemblyInstruction>> generateAssembly(int numberOfStates, std::vector<std::unique_ptr<Transition>> &transitions, int startingInstruction);