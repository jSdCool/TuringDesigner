#pragma once

#include <deque>
#include <string>

class Tape {
    std::deque<char> buffer;
    int headPosition = 0;
public:
    explicit Tape(const std::string &input);

    void left();
    void right();
    [[nodiscard]] char read() const;
    void write(char c);
    [[nodiscard]] size_t size() const;
    char operator[](size_t index) const;
    /**Get the current position of the head
     * @return The index the head is pointing to
     */
    size_t operator*() const;
};

