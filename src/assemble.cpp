#include "assemble.hpp"

#include <utility>
#include <fstream>
#include "helper.hpp"

class AlphaInstruction : public AssemblyInstruction {
    const char letter;
    const bool offFlag;
public:
    //wow c++ is weird
    explicit AlphaInstruction(char letter, bool off) : AssemblyInstruction(0), letter(letter), offFlag(off) {}

    std::string getAssembly() override {
        std::string result = "alpha '~'";
        result[7] = letter;
        if (offFlag) {
            result += " 1";
        }
        return result;
    }
};

class CompareInstruction : public AssemblyInstruction {
    const char letter;
    const bool blankFlag;
    const bool orFlag;
public:
    explicit CompareInstruction(char letter, bool isBlank, bool orOp): AssemblyInstruction(1),
        letter(letter), orFlag(orOp), blankFlag(isBlank) {}
    std::string getAssembly() override {
        std::string result = "compare '~'";

        if (blankFlag) {
            result[9] = '0';
            result += " 1";
        } else {
            result[9] = letter;
            result += " 0";
        }
        if (orFlag) {
            result += " 1";
        } else {
            result += " 0";
        }
        //comment
        result += " ; ";
        if (orFlag) {
            result += "or ";
        } else {
            result += "compare ";
        }
        if (blankFlag) {
            result+= "BLANK";
        } else {
            result += "v";
            result[result.length()-1] = letter;
        }
        return result;
    }
    //alternate constructor to automatically determine the blank-ness
    CompareInstruction(char letter, bool orOp) :AssemblyInstruction(1), letter(letter == BLANK_CHAR ? '0' : letter), blankFlag(letter == BLANK_CHAR ), orFlag(orOp) {}
};

class JumpEqualInstruction : public AssemblyInstruction {
    const std::string label;
public:
    explicit JumpEqualInstruction(std::string label) : AssemblyInstruction(2), label(std::move(label)) {}
    std::string getAssembly() override {
        return "je !"+label;
    }
};

class JumpNotEqualInstruction : public AssemblyInstruction {
    const std::string label;
public:
    explicit JumpNotEqualInstruction(std::string label) : AssemblyInstruction(3), label(std::move(label)) {}
    std::string getAssembly() override {
        return "jne !"+label;
    }
};

class JumpInstruction : public AssemblyInstruction {
    const std::string label;
    const std::string comment;
public:
    explicit JumpInstruction(std::string label, std::string comment) : AssemblyInstruction(4), label(std::move(label)),comment(std::move(comment)) {}
    std::string getAssembly() override {
        std::string resultCmt;
        if (!comment.empty()) {
            resultCmt = " ; "+comment;
        }
        return "jmp !"+label+resultCmt;
    }
};

class PutInstruction : public AssemblyInstruction {
    const char letter;
    const bool blankFlag;
public:
    explicit PutInstruction(char latter, bool isBlank) : AssemblyInstruction(5), letter(latter), blankFlag(isBlank) {}
    std::string getAssembly() override {
        std::string result = "put '~'";
        if (blankFlag) {
            result[5] = '0';
            result += " 1";
        } else {
            result[5] = letter;
        }
        return result;
    }
    PutInstruction(char letter): AssemblyInstruction(5), letter(letter == BLANK_CHAR ? '0' : letter), blankFlag(letter == BLANK_CHAR ) {}
};

class MoveInstruction : public AssemblyInstruction {
    const int amount;
    const bool haltSuccess;
    const std::string comment;
public:
    MoveInstruction(int amount,bool halt, std::string comment): AssemblyInstruction(6), amount(amount), haltSuccess(halt), comment(std::move(comment)) {}
    std::string getAssembly() override {
        std::string result = "move ";
        result += std::to_string(amount);
        if (amount == 0 && haltSuccess) {
            result += " 1";
        }
        result += " ; "+comment;
        return result;
    }

    static MoveInstruction * left() {
        return new MoveInstruction(-1,false,"left");
    }
    static MoveInstruction * right() {
        return new MoveInstruction(1,false,"right");
    }
    static MoveInstruction * fail() {
        return new MoveInstruction(0,false,"fail");
    }
    static MoveInstruction * halt() {
        return new MoveInstruction(0,true,"halt");
    }
};

class LabelSudoInstruction : public AssemblyInstruction {
    const std::string label;
    const std::string comment;
public:
    explicit LabelSudoInstruction(std::string label, std::string comment) : AssemblyInstruction(-1), label(std::move(label)), comment(std::move(comment)) {}
    std::string getAssembly() override {
        std::string resultCmt;
        if (!comment.empty()) {
            resultCmt = " ; "+comment;
        }
        return "!"+label+resultCmt;
    }
};

std::string generateTransitionLabel(Transition * transition) {
    std::string result = std::to_string(transition->getStartIndex()+1);
    result += "_";
    std::string match = transition->getMatchRule();
    if (match[0] == '[') {
        for (size_t i=1;i<match.length()-1;i++) {
            if (match[i]!=BLANK_CHAR) {
                result += "'";
                result[result.length()-1] = match[i];
            }else {
                result += "_BLANK";
            }
        }
    } else {
        if (match[0]!=BLANK_CHAR) {
            result += "'";
            result[result.length()-1] = match[0];
        } else {
            result+="BLANK";
        }
    }
    return result;
}

std::vector<std::unique_ptr<AssemblyInstruction>> generateAssembly(int numberOfStates,std::vector<std::unique_ptr<Transition>> &transitions, int startingInstruction, std::string &alphabet) {
    std::vector<std::unique_ptr<AssemblyInstruction>> result;

    //add the alphabet to the assembly instructions
    for (char letter : alphabet) {
        result.emplace_back(std::make_unique<AlphaInstruction>(letter,false));
    }

    result.emplace_back(std::make_unique<JumpInstruction>(std::to_string(startingInstruction+1),"go to start state"));

    for (int i=0; i<numberOfStates; i++) {
        //for each state:
        //add the label for this state
        result.emplace_back(std::make_unique<LabelSudoInstruction>(std::to_string(i+1),"state "+std::to_string(i+1)));

        //find all the necessary transitions
        std::vector<Transition*> stateTransitions;
        for (std::unique_ptr<Transition> & transition : transitions) {
            if (transition->getStartIndex() == i) {
                Transition * tp = transition.get();
                if (tp == nullptr) {
                    std::cerr << "ALERT! Unique Pointer get returned null!!!" << std::endl;
                }
                stateTransitions.push_back(tp);//another fun fake MSVC error
            }
        }

        for (Transition * transition : stateTransitions) {
            std::string condition = transition->getMatchRule();
            //if we find a raw wild card transition
            if (condition[0] == '*') {
                //just wright in the entire transition here
                char write = transition->getWright();
                if (write != '*') {//if we are writing something then wright it
                    result.emplace_back(std::make_unique<PutInstruction>(write));
                }
                //move the head
                if (transition->getMove() == 'L') {
                    result.emplace_back(MoveInstruction::left());
                } else {
                    result.emplace_back(MoveInstruction::right());
                }
                if (transition->isHalt()) {
                    result.emplace_back(MoveInstruction::halt());
                    break;
                }
                result.emplace_back(std::make_unique<JumpInstruction>(std::to_string(transition->getEndIndex()+1),"go to state "+std::to_string(transition->getEndIndex()+1)));
                break;
                //no need to process further transitions
            }
            //if it is a complex transition
            if (condition[0] == '[') {
                bool invert = condition[1] == '^';
                bool useOr = false;
                //for each letter to compare
                for (size_t letterP = invert?2:1;letterP < condition.length()-1;letterP++) {
                    result.emplace_back(std::make_unique<CompareInstruction>(condition[letterP],useOr));
                    useOr = true;
                }
                if (invert) {
                    //jump when not equal
                    result.emplace_back(std::make_unique<JumpNotEqualInstruction>(generateTransitionLabel(transition)));
                }else {
                    //jump when equal
                    result.emplace_back(std::make_unique<JumpEqualInstruction>(generateTransitionLabel(transition)));
                }

            } else {
                //compare instruction
                result.emplace_back(std::make_unique<CompareInstruction>(condition[0],false));
                //jump is equal instruction
                result.emplace_back(std::make_unique<JumpEqualInstruction>(generateTransitionLabel(transition)));
            }
        }
        //add the fail condition to the end of the state, if this is reached it means none of the transitions were valid and therefore the machine fails
        result.emplace_back(MoveInstruction::fail());

        //each transition specific use
        for (Transition * transition : stateTransitions) {
            if (transition->getMatchRule()[0] != '*') {//dont bother writing the wild card transitions
                //put in the transition label
                result.emplace_back(std::make_unique<LabelSudoInstruction>(generateTransitionLabel(transition),""));
                char write = transition->getWright();
                if (write != '*') {//if we are writing something then wright it
                    result.emplace_back(std::make_unique<PutInstruction>(write));
                }
                //move the head
                if (transition->getMove() == 'L') {
                    result.emplace_back(MoveInstruction::left());
                } else {
                    result.emplace_back(MoveInstruction::right());
                }
                //jump to the next transition
                if (transition->isHalt()) {
                    //or halt if it is a halt transition
                    result.emplace_back(MoveInstruction::halt());
                } else {
                    result.emplace_back(std::make_unique<JumpInstruction>(std::to_string(transition->getEndIndex()+1),"go to state "+std::to_string(transition->getEndIndex()+1)));
                }
            }
        }

    }

    return result;
}

void optimizeAssembly( std::vector<std::unique_ptr<AssemblyInstruction>> &assembly_instructions) {
    //TODO maby
    //remove the jump to start state if the start state is the first label in the file
    //remove unconditional jumps if the next line is the label it jumps to
    //if the instruction right adder a label is a fail or halt, replace all non conditional jumps to that label with the fail / halt instruction
    //if a label is used only once for a single unconditional jump, inline that label
    //if a fail / halt immediately follows an unconditioned jump, remove it
    //if a fail / halt immediately follow a fail / halt, get rid of it
}

void writeAssemblyToFile(const std::vector<std::unique_ptr<AssemblyInstruction>>& assembly_instructions, std::string &filename) {
    std::ofstream file(filename);
    for (const std::unique_ptr<AssemblyInstruction> &instruction : assembly_instructions) {
        file << instruction->getAssembly() << std::endl;
    }
    file.close();
}
