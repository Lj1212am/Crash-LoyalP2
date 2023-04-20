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

#include "Controller_AI_KevinDill.h"

#include "Constants.h"
#include "EntityStats.h"
#include "iPlayer.h"
#include "Vec2.h"




Controller_AI_KevinDill::Controller_AI_KevinDill() : m_behaviorTree(createBehaviorTree())
{

    actionMap = {
        {"placemobs", &Controller_AI_KevinDill::PlaceMobs},
        {"deployarchersandswordsman", &Controller_AI_KevinDill::DeployArchersandSwordsman},
        {"deploylanepressure", &Controller_AI_KevinDill::DeployLanePressure},
        {"defendcounterattack", &Controller_AI_KevinDill::DefendCounterAttack},
        {"deploygiantforrogueretrieval", &Controller_AI_KevinDill::DeployGiantForRogueRetrieval}
    };

    // Initializes random seed when creating controller
    srand(time(0));
    while (true)
    {
        std::cout << "Enter a new behavior (type 'quit' to exit): ";
        std::string input;
        std::getline(std::cin, input);

        if (input == "quit")
        {
            break;
        }

        learnBehavior(input);
    }
}

Controller_AI_KevinDill::Node Controller_AI_KevinDill::createBehaviorTree()
{
    // Creates the root node
    Node root(NodeType::Selector);

    // Creates a sequence node containing an action node child for deploying a giant for rogue retrieval
    Node rogueRetrievalSequence(NodeType::Sequence);
    rogueRetrievalSequence.children.emplace_back(NodeType::Action);
    rogueRetrievalSequence.children.back().action = &Controller_AI_KevinDill::DeployGiantForRogueRetrieval;

    // Creates a sequence node containing an action node child for placing mobs
    Node placeMobsSequence(NodeType::Sequence);
    placeMobsSequence.children.emplace_back(NodeType::Action);
    placeMobsSequence.children.back().action = &Controller_AI_KevinDill::PlaceMobs;

    // Creates a sequence node containing an action node child for deploying the archers and swordsman
    Node archerSwordsmanSequence(NodeType::Sequence);
    archerSwordsmanSequence.children.emplace_back(NodeType::Action);
    archerSwordsmanSequence.children.back().action = &Controller_AI_KevinDill::DeployArchersandSwordsman;

    // Creates a sequence node containing an action node child for deploying a giant and rogue for lane pressure
    Node lanePressureSequence(NodeType::Sequence);
    lanePressureSequence.children.emplace_back(NodeType::Action);
    lanePressureSequence.children.back().action = &Controller_AI_KevinDill::DeployLanePressure;

    // Creates a sequence node containing an action node child for using swordsmen to defend attacks on towers
    Node defendCounterAttackSequence(NodeType::Sequence);
    defendCounterAttackSequence.children.emplace_back(NodeType::Action);
    defendCounterAttackSequence.children.back().action = &Controller_AI_KevinDill::DefendCounterAttack;

    // Adds all the sequence nodes to the root node
    root.children.push_back(defendCounterAttackSequence);
    root.children.push_back(rogueRetrievalSequence);
    root.children.push_back(placeMobsSequence);
    root.children.push_back(archerSwordsmanSequence);
    root.children.push_back(lanePressureSequence);

    return root;
}

bool Controller_AI_KevinDill::traverseTree(const Node& node)
{
    // Recursive function used to traverse tree
    // Logic depends on the node type
    switch (node.type)
    {
    case NodeType::Selector:
        for (const Node& child : node.children)
        {
            if (traverseTree(child))
                return true;
        }
        return false;

    case NodeType::Sequence:
        for (const Node& child : node.children)
        {
            if (!traverseTree(child))
                return false;
        }
        return true;

    case NodeType::Action:
        if (node.action)
            return (this->*node.action)();
    }

    return false;
}

void Controller_AI_KevinDill::tick(float deltaTSec)
{
    // Every tick traverse the behavior tree
    traverseTree(m_behaviorTree);
}

bool Controller_AI_KevinDill::PlaceMobs()
{
    assert(m_pPlayer);

    // Wait for elixir
    if (m_pPlayer->getElixir() >= 9)
    {
        float kx = rand() % 2 == 0 ? LEFT_BRIDGE_CENTER_X : RIGHT_BRIDGE_CENTER_X;
        static const Vec2 ksGiantPos(kx, RIVER_TOP_Y - 0.5f);
        static const Vec2 ksRoguePos(kx, RIVER_TOP_Y - 1.5f);
        static const Vec2 ksArcherPos(kx, 0.f);

        // Convert the positions from player space to game space
        bool isNorth = m_pPlayer->isNorth();
        Vec2 giantPos_Game = ksGiantPos.Player2Game(isNorth);
        Vec2 archerPos_Game = ksArcherPos.Player2Game(isNorth);
        Vec2 roguePos_Game = ksRoguePos.Player2Game(isNorth);

        // Create two archers and a giant
        m_pPlayer->placeMob(iEntityStats::Giant, giantPos_Game);
        m_pPlayer->placeMob(iEntityStats::Archer, archerPos_Game);
        m_pPlayer->placeMob(iEntityStats::Rogue, roguePos_Game);
        
        return true;
    }

    return false;
    
}

bool Controller_AI_KevinDill::DeployArchersandSwordsman()
{
    const iEntityStats& archerStats = iEntityStats::getStats(iEntityStats::Archer);
    const iEntityStats& swordmanStats = iEntityStats::getStats(iEntityStats::Swordsman);

    float totalCost = 2 *  archerStats.getElixirCost() +   swordmanStats.getElixirCost();
    

    bool isNorth = m_pPlayer->isNorth();

    // Wait for elixir
    if (m_pPlayer->getElixir() >= totalCost)
    {
        int probability = rand() % 100;

        printf("checking for lane pressure probablitiy score: %d\n", probability);
        
        // 40% chance to deploy lane pressure units
        if (probability < 40)
        {

            int leftLaneCount, rightLaneCount;

            // Get the amount of opposing units in each lane and deploy to that lane
            getLaneTroopCounts(leftLaneCount, rightLaneCount, true);

            float x = leftLaneCount >= rightLaneCount ? LEFT_BRIDGE_CENTER_X : RIGHT_BRIDGE_CENTER_X;
            float yOffset = m_pPlayer->isNorth() ? RIVER_TOP_Y - 2.0f : RIVER_BOT_Y + 2.0f;

            static const Vec2 ksArcherPos(x, 0.f);

            // Deplor archers in the back and side by side
            Vec2 archerPos_Game = ksArcherPos.Player2Game(isNorth);
            archerPos_Game.x = x;
            Vec2 archerPos_Game1 = archerPos_Game;
            archerPos_Game.x += 0.5f;
            m_pPlayer->placeMob(iEntityStats::Archer, archerPos_Game);
            m_pPlayer->placeMob(iEntityStats::Archer, archerPos_Game1);

            // Deploy swordsman
            Vec2 swordsmanPos1_Game(x - 1.0f, yOffset);
            m_pPlayer->placeMob(iEntityStats::Swordsman, swordsmanPos1_Game);
            

            return true;
        }
    }
    return false;
}

void Controller_AI_KevinDill::getLaneTroopCounts(int& leftLaneCount, int& rightLaneCount, bool opp = false)
{
    leftLaneCount = 0;
    rightLaneCount = 0;

    // If counting opponent's troops or not.
    if (opp)
    {
        // Count all of the opponents mobs in each lane
        for (int i = 0; i < m_pPlayer->getNumOpponentMobs(); ++i)
        {
            iPlayer::EntityData mob = m_pPlayer->getOpponentMob(i);
            if (mob.m_Position.x < GAME_GRID_WIDTH / 2)
            {
                leftLaneCount++;
            }
            else
            {
                rightLaneCount++;
            }
        }
    }
    else
    {
        for (int i = 0; i < m_pPlayer->getNumMobs(); ++i)
        {
            // Count all of the AI's mobs in each lane
            iPlayer::EntityData mob = m_pPlayer->getMob(i);
            if (mob.m_Position.x < GAME_GRID_WIDTH / 2)
            {
                leftLaneCount++;
            }
            else
            {
                rightLaneCount++;
            }
        }
    }
    
}

bool Controller_AI_KevinDill::DeployLanePressure()
{

    const iEntityStats& giantStats = iEntityStats::getStats(iEntityStats::Giant);
    const iEntityStats& rogueStats = iEntityStats::getStats(iEntityStats::Rogue);

    float totalCost = giantStats.getElixirCost() + rogueStats.getElixirCost();


    bool isNorth = m_pPlayer->isNorth();

    // Wait for Elixir
    if (m_pPlayer->getElixir() >= totalCost)
    {
        int probability = rand() % 100;
        printf("checking for Giant + Rogue probablitiy score: %d\n", probability);
        
        // 30% chance to deploy giant and rogue
        if (probability < 30) 
        {
            int leftLaneCount, rightLaneCount;

            // Deploy Giant and rogue to lane with the least amount of friendly troops for lane pressure.
            getLaneTroopCounts(leftLaneCount, rightLaneCount);

            float x = leftLaneCount <= rightLaneCount ? LEFT_BRIDGE_CENTER_X : RIGHT_BRIDGE_CENTER_X;

            float yOffset = m_pPlayer->isNorth() ? RIVER_TOP_Y - 1.0f : RIVER_BOT_Y + 1.0f;

            // Deploy Giant and Rogue behind.
            Vec2 giantPos(x, yOffset);
            Vec2 giantPos_Game = giantPos.Player2Game(isNorth);
            m_pPlayer->placeMob(iEntityStats::Giant, giantPos_Game);

            Vec2 roguePos(x, yOffset + (m_pPlayer->isNorth() ? -(rogueStats.getHideDistance() + (giantStats.getSize() / 2.f)) : (rogueStats.getHideDistance() + (giantStats.getSize() / 2.f))));
            Vec2 roguePos_Game = roguePos.Player2Game(isNorth);
            m_pPlayer->placeMob(iEntityStats::Rogue, roguePos_Game);

            return true;
        }
    }
    return false;
}

bool Controller_AI_KevinDill::DefendCounterAttack()
{
    // How close enemy mobs need to be to towers to trigger defense
    const float defenseThreshold = 6.0f;
    const float unitSpawnOffset = 1.5f;
    
    // Loop through each opposing unit
    for (int i = 0; i < m_pPlayer->getNumOpponentMobs(); i++)
    {
        iPlayer::EntityData mob = m_pPlayer->getOpponentMob(i);
        Vec2 mobPos = mob.m_Position;

        float minDistanceToTower = INFINITY;
        Vec2 closestTowerPos;

        // Check distances to all three towers
        for (int j = 0; j < m_pPlayer->getNumBuildings(); j++)
        {
            iPlayer::EntityData tower = m_pPlayer->getBuilding(j);
            Vec2 towerPos = tower.m_Position;

            float distance = (mobPos - towerPos).length();
            if (distance < minDistanceToTower)
            {
                minDistanceToTower = distance;
                closestTowerPos = towerPos;
            }
        }

        // If an enemy mob is too close to the tower, deploy a defensive unit
        if (minDistanceToTower <= defenseThreshold)
        {
            {
                float unitCost = iEntityStats::getStats(iEntityStats::Swordsman).getElixirCost();
                if (m_pPlayer->getElixir() >= unitCost)
                {
                    Vec2 spawnPos = closestTowerPos + Vec2(unitSpawnOffset, 0.f);
                    m_pPlayer->placeMob(iEntityStats::Swordsman, spawnPos);
                    return true;
                }
            }
        }
    }

    return false;
}

bool Controller_AI_KevinDill::DeployGiantForRogueRetrieval()
{
    const float rogueHideDistance = iEntityStats::getStats(iEntityStats::Rogue).getHideDistance();
    const float giantCost = iEntityStats::getStats(iEntityStats::Giant).getElixirCost();

    // Wait for elixir
    if (m_pPlayer->getElixir() >= giantCost)
    {
        // If a rogue is hiding behind one of the friendly towers
        // place a giant within it's PreferGiantSight
        for (int i = 0; i < m_pPlayer->getNumMobs(); ++i)
        {
            iPlayer::EntityData mob = m_pPlayer->getMob(i);
            if (mob.m_Stats.getMobType() == iEntityStats::Rogue)
            {
                for (int j = 0; j < m_pPlayer->getNumBuildings(); ++j)
                {
                    iPlayer::EntityData building = m_pPlayer->getBuilding(j);
                    float distance = (mob.m_Position - building.m_Position).length();
                    if (distance <= (0.5 + rogueHideDistance + building.m_Stats.getSize() / 2.f))
                    {
                       
                        Vec2 giantPos = mob.m_Position + Vec2(1.5f, 0.0f);
                        m_pPlayer->placeMob(iEntityStats::Giant, giantPos);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

Controller_AI_KevinDill::Node Controller_AI_KevinDill::parseNaturalText(const std::string& behaviorText)
{
    std::istringstream iss(behaviorText);
    std::string word;
    Node root(NodeType::Selector);
    std::stack<std::pair<Node*, bool>> nodeStack;
    nodeStack.push({ &root, false });

    while (iss >> word)
    {
        // Convert the word to lowercase
        std::transform(word.begin(), word.end(), word.begin(), [](unsigned char c) { return std::tolower(c); });

        if (word == "if")
        {
            Node selectorNode(NodeType::Selector);
            nodeStack.top().first->children.push_back(selectorNode);
            nodeStack.push({ &nodeStack.top().first->children.back(), false });
        }
        else if (word == "and")
        {
            Node sequenceNode(NodeType::Sequence);
            nodeStack.top().first->children.push_back(sequenceNode);
            nodeStack.push({ &nodeStack.top().first->children.back(), false });
        }
        else if (word == "end")
        {
            if (!nodeStack.empty())
            {
                nodeStack.top().second = true;
                while (!nodeStack.empty() && nodeStack.top().second)
                {
                    nodeStack.pop();
                }
            }
        }
        else
        {
            // Check if the word is a substring of any key in the actionMap
            for (const auto& pair : actionMap)
            {
                if (pair.first.find(word) != std::string::npos)
                {
                    Node actionNode(NodeType::Action);
                    actionNode.action = pair.second;
                    nodeStack.top().first->children.push_back(actionNode);
                    break;
                }
            }
        }
    }

    return root;
}









std::string Controller_AI_KevinDill::treeToString(const Node& node, int depth) const
{
    std::string output;

    for (int i = 0; i < depth; i++)
        output += "  ";

    switch (node.type)
    {
    case NodeType::Selector:
        output += "Selector\n";
        break;
    case NodeType::Sequence:
        output += "Sequence\n";
        break;
    case NodeType::Action:
        output += "Action: ";

        for (const auto& pair : actionMap)
        {
            if (node.action == pair.second)
            {
                output += pair.first;
                break;
            }
        }

        output += "\n";
        break;
    }

    for (const auto& child : node.children)
        output += treeToString(child, depth + 1);

    return output;
}

void Controller_AI_KevinDill::learnBehavior(const std::string& behaviorText)
{
    Node newBehaviorTree = parseNaturalText(behaviorText);
    m_behaviorTree = newBehaviorTree;
    std::string treeRepresentation = treeToString(m_behaviorTree, 0);
    std::cout << treeRepresentation;
}