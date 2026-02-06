#pragma once
#include <stdexcept>
#include <string>
#include <vector>

#include "raylib.h"
#include "State.h"

struct TransitionCreateInfo {
    int startIndex = -1;
    int endIndex = -1;
    std::vector<Vector2> points{};
    std::string match = "X";
    char wright='\0';
    char move='\0';
};

class Transition {
protected:
    int fromState;
    int toState;

    std::vector<Vector2> midPoints;
    std::string match="X";
    char wright = 'X';
    char move = 'X';
public:
    virtual ~Transition() = default;

    Transition(const int from, const int to, const std::vector<Vector2>& mid) {
        fromState = from;
        toState = to;

        if (mid.empty() || mid.size() > 2) {
            throw std::runtime_error("Invalid number of transition points");
        }
        midPoints = mid;
    }

    explicit Transition(const TransitionCreateInfo& createInfo) {
        fromState = createInfo.startIndex;
        toState = createInfo.endIndex;
        if (createInfo.points.empty() || createInfo.points.size() > 2) {
            throw std::runtime_error("Invalid number of transition points");
        }
        midPoints = createInfo.points;
        match = createInfo.match;
        wright = createInfo.wright;
        move = createInfo.move;

    }

    virtual void draw(float scale, std::vector<State> &states, Vector2 offset,bool highlight) const;

    void setTransitionRules(std::string &match, char wright, char move);

    std::string getMatchRule();
    [[nodiscard]] char getWright() const;
    [[nodiscard]] char getMove() const;
    virtual bool mouseOverText(Vector2 mouse, std::vector<State> &states);

    [[nodiscard]] int getStartIndex() const {
        return fromState;
    }
    [[nodiscard]] int getEndIndex() const {
        return toState;
    }
    void decreaseStartIndex() {
        fromState--;
    }
    void decreaseEndIndex() {
        toState--;
    }

    std::vector<Vector2> getMidPoints() {
        return midPoints;
    }
    void setMidPoint(Vector2 midPoint, int index) {
        midPoints[index] = midPoint;
    }

};

class HaltTransition : public Transition {
public:
    explicit HaltTransition(const TransitionCreateInfo& createInfo) : Transition(createInfo) {
        fromState = createInfo.startIndex;
        midPoints = createInfo.points;
        match = createInfo.match;
        wright = createInfo.wright;
        move = createInfo.move;
        toState = -1;
    }

    void draw(float scale, std::vector<State> &states, Vector2 offset,bool highlight) const override;

    bool mouseOverText(Vector2 mouse, std::vector<State> &states) override;

};