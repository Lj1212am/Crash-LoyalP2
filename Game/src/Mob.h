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
    
    // This function checks whether a mob is currently hiding.
    // A mob is hiding if all the opposing entities can't see it.
    bool isHiding() const;
    
    // A function that checks with the mob is obstructed from the entity by a tower or Giant.
    bool isObstructedByGiantOrTower(Entity* e, Player& friendlyPlayer) const;

    // Checks if a line intersects with a square.
    bool lineSquareIntersection(Vec2 start, float size, Vec2 obj_pos) const;

    // Function to determing if the mob  has a target in spring attack range.
    bool isEnemyInSpringAttackRange();

    // Function to determing if the mob would prefer the Giant in Range 
    // compared to a tower.
    bool friendlyGiantPreferRange();

    // Function to get the hiding location that is around the object the rogue is hiding behind.
    Vec2 getHidingLocation(Entity* friendlyObject);

private:
    const Vec2* m_pWaypoint;

    // Counts amount of ticks since last hidden.
    int m_ticksSinceHidden = 0;

    // Flag for if the mob is following a giant.
    bool m_bFollowingGiant = false;

    // The friendly giant the mob is following.
    Entity* m_eFriendlyGiant;

    // Flag for if the mob is following a building.
    bool m_bFollowingBuilding = false;


    // The friendly building the mob is following.
    Entity* m_eFriendlyBuilding;

    
};
