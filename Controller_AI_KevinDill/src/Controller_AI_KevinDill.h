// MIT License
// 
// Copyright(c) 2020 Kevin Dill
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <stack>
#include "iController.h"
#include <vector>


class Controller_AI_KevinDill : public iController
{
public:

    // Three types of Nodes in a behavior tree
    // Selector :: if
    // Sequence :: and
    // Action :: leaf node function
    enum class NodeType
    {
        Selector,
        Sequence,
        Action
    };

    // Each node is made up of a type
    // a list of it's children
    // and a pointer to a function the represents an action
    struct Node
    {
        NodeType type;
        std::vector<Node> children;
        bool (Controller_AI_KevinDill::* action)();

        // Node constructor
        Node(NodeType t) : type(t), action(nullptr) {}
    };

    Controller_AI_KevinDill();
    virtual ~Controller_AI_KevinDill() {}

    // A method to learn behavior from text input.
    void learnBehavior(const std::string& behaviorText);

    void tick(float deltaTSec);

    int GetFoo() const { return m_foo; }


private:
    int m_foo = 0;

    // This map associates natural text commands with corresponding AI actions.
    std::map<std::string, bool (Controller_AI_KevinDill::*)()> actionMap;

    // The Ai's Behavior Tree.
    Node m_behaviorTree;
    
    // Function that creates the behavior tree for the AI.
    Node createBehaviorTree();

    // Function that traverses the behavior tree every tick.
    bool traverseTree(const Node& node);

    // Places the initial mobs, keeping the functionality from the original AI.
    bool PlaceMobs();

    // Deploys two Archers and a Swordsman to the lane with the most enemy units.
    bool DeployArchersandSwordsman();

    // Counts the amount of troops in each side of the map, either friendly or not.
    void getLaneTroopCounts(int& leftLaneCount, int& rightLaneCount, bool opp);

    // Places a Giant and a Rogue in the lane with the least amount of friendly troops for lane pressure.
    bool DeployLanePressure();

    // Defends attacks on the towers with swordsmen.
    bool DefendCounterAttack();

    // If the AI has rogues hiding behind towers, it will place a Giant in range for the Rogues to follow.
    bool DeployGiantForRogueRetrieval();

    // This function takes a natural language input describing a behavior and constructs a behavior tree based on the given text.
    Node parseNaturalText(const std::string& behaviorText);

    // Prints out the construction of a behavior tree.
    std::string treeToString(const Node& node, int depth) const;
};