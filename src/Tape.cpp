#include "Tape.hpp"

#include <stdexcept>

#include "helper.hpp"

Tape::Tape(const std::string &input) {
    headPosition = 0;
    for (size_t i = 0; i < input.length(); i++) {
        buffer.push_back(input[i]);//MSVC analyzer shows an error here but, it builds just fine, thanks microslop
    }
}

void Tape::left() {
    if (headPosition == 0) {//if at the start of the tape, and we try to move left,
        buffer.push_front(BLANK_CHAR);//add a new blank char to the start of the tape and do no change the tape position
    } else {
        headPosition--;
    }
}

void Tape::right() {
    if (headPosition == buffer.size() - 1) {//if we are at the end of the tape
        buffer.push_back(BLANK_CHAR);//add a new blank char to the end of the tape
    }
    headPosition++;
}

char Tape::read() const {
    return buffer[headPosition];
}

void Tape::write(char c) {
    buffer[headPosition] = c;
}

size_t Tape::size() const {
    return buffer.size();
}

char Tape::operator[](size_t index) const {
    if (index >= size()) {
        throw std::out_of_range("Provided index out of range (provided: "+std::to_string(index)+" size: "+std::to_string(size())+")");
    }
    return buffer[index];
}

size_t Tape::operator*() const {
    return headPosition;
}
