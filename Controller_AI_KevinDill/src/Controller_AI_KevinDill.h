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

#include "iController.h"
#include <vector>


class Controller_AI_KevinDill : public iController
{
public:

    enum class NodeType
    {
        Selector,
        Sequence,
        Action
    };

    struct Node
    {
        NodeType type;
        std::vector<Node> children;
        bool (Controller_AI_KevinDill::* action)();

        Node(NodeType t) : type(t), action(nullptr) {}
    };

    Controller_AI_KevinDill();
    virtual ~Controller_AI_KevinDill() {}

    void tick(float deltaTSec);

    int GetFoo() const { return m_foo; }


private:
    int m_foo = 0;

    Node m_behaviorTree;

    Node createBehaviorTree();
    bool traverseTree(const Node& node);

    // Define your custom action functions here
    bool checkElixirAndPlaceMobs();
    bool checkElixirAndDeployGiantAndRogue();
    bool checkElixirAndDeployLanePressure();
    bool checkElixirAndDefendCounterAttack();
    bool checkElixirAndDeployGiantForRogueRetrieval();
};