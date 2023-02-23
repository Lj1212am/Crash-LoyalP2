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

#include "Entity.h"
#include "Player.h"

struct Waypoint;

class Mob : public Entity {

public:
    Mob(const iEntityStats& stats, const Vec2& pos, bool isNorth);

    virtual void tick(float deltaTSec);

    virtual bool isHidden() const;
    

protected:
    void move(float deltaTSec);
    const Vec2* pickWaypoint();
    Mob* checkCollision();
    void processCollision(Mob* otherMob, float deltaTSec);
    bool isHiding() const;
    bool isObstructedByGiantOrTower(Entity* e, Player& friendlyPlayer) const;
    bool lineSquareIntersection(Vec2 start, float size, Vec2 obj_pos) const;
    const Vec2* pickRogueWaypoint();
    bool isEnemyInSpringAttackRange();


private:
    const Vec2* m_pWaypoint;
    int m_ticksSinceHidden = 0;
    bool m_bFollowingGiant = false;
    Entity* m_eFriendlyGiant;

    
};
