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

#include "Mob.h"

#include "Constants.h"
#include "Game.h"


#include <algorithm>
#include <vector>


Mob::Mob(const iEntityStats& stats, const Vec2& pos, bool isNorth)
    : Entity(stats, pos, isNorth)
    , m_pWaypoint(NULL)
{
    assert(dynamic_cast<const iEntityStats_Mob*>(&stats) != NULL);
}

void Mob::tick(float deltaTSec)
{
    // Tick the entity first.  This will pick our target, and attack it if it's in range.
    Entity::tick(deltaTSec);

    // if our target isn't in range, move towards it.
    if (!targetInRange())
    {
        move(deltaTSec);
    }
}

bool lineLineIntersect(Vec2 p1, Vec2 p2, Vec2 q1, Vec2 q2, Vec2& intersection)
{
    float x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y;
    float x3 = q1.x, y3 = q1.y, x4 = q2.x, y4 = q2.y;

    float denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
    if (denom == 0.f) {
        return false;  // lines are parallel
    }

    float t1 = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denom;
    float t2 = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denom;

    if (t1 >= 0.f && t1 <= 1.f && t2 >= 0.f && t2 <= 1.f) {
        intersection.x = x1 + t1 * (x2 - x1);
        intersection.y = y1 + t1 * (y2 - y1);
        return true;
    }
    else {
        return false;  // line segments do not intersect
    }
}


bool Mob::lineSquareIntersection(Vec2 start, float size, Vec2 obj_pos) const
{
    Vec2 direction = start - m_Pos;
    Vec2 topLeft = Vec2(obj_pos.x - (size / 2.0f), obj_pos.y - (size / 2.0f));
    Vec2 topRight = Vec2(obj_pos.x + (size / 2.0f), obj_pos.y - (size / 2.0f));
    Vec2 bottomLeft = Vec2(obj_pos.x - (size / 2.0f), obj_pos.y + (size / 2.0f));
    Vec2 bottomRight = Vec2(obj_pos.x + (size / 2.0f), obj_pos.y + (size / 2.0f));

    // check intersection with top side
    Vec2 intersection;
    if (lineLineIntersect(start, m_Pos, topLeft, topRight, intersection)) {
        if (intersection.x >= topLeft.x && intersection.x <= topRight.x) {
            return true;
        }
    }

    // check intersection with right side
    if (lineLineIntersect(start, m_Pos, topRight, bottomRight, intersection)) {
        if (intersection.y >= topRight.y && intersection.y <= bottomRight.y) {
            return true;
        }
    }

    // check intersection with bottom side
    if (lineLineIntersect(start, m_Pos, bottomLeft, bottomRight, intersection)) {
        if (intersection.x >= bottomLeft.x && intersection.x <= bottomRight.x) {
            return true;
        }
    }

    // check intersection with left side
    if (lineLineIntersect(start, m_Pos, topLeft, bottomLeft, intersection)) {
        if (intersection.y >= topLeft.y && intersection.y <= bottomLeft.y) {
            return true;
        }
    }

    // no intersection found
    return false;
}








//    Vec2 t_near;
//    Vec2 t_far;
//    t_near.x = (top_left.x - start.x) / direction.x;
//    t_near.y = (top_left.y - start.y) / direction.y;
//    t_far.x = (top_left.x + size - start.x) / direction.x;
//    t_far.y = (top_left.y + size - start.y) / direction.y;
//    
//    //if (t_near.x > t_far.x)
//    //{
//    //    float tmp_x = t_near.x;
//    //    t_near.x = t_far.x;
//    //    t_far.x = tmp_x;
//    //}
//    //if (t_near.y > t_far.y)
//    //{
//    //    float tmp_y = t_near.y;
//    //    t_near.y = t_far.y;
//    //    t_far.y = tmp_y;
//    //}
//    if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
//    if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);
//
//    if (t_near.x > t_far.y || t_near.y > t_far.x)
//    {
//        return false;
//    }
//
//    float t_hit_near = std::max(t_near.x, t_near.y);
//    float t_hit_far = std::min(t_far.x, t_far.y);
//
//    if (t_hit_far < 0)
//    {
//        return false;
//    }
//
//    return true;
//
//}

bool Mob::isObstructedByGiantOrTower(Entity* e, Player& friendlyPlayer) const
{
    Vec2 direction = e->getPosition() - m_Pos;
    float distance = direction.length();

    std::vector<Entity*> playerEntities;
    playerEntities.insert(playerEntities.end(), friendlyPlayer.getMobs().begin(), friendlyPlayer.getMobs().end());
    playerEntities.insert(playerEntities.end(), friendlyPlayer.getBuildings().begin(), friendlyPlayer.getBuildings().end());

    for (Entity* entity : friendlyPlayer.getBuildings())
    {
        if(lineSquareIntersection(e->getPosition(), entity->getStats().getSize(), entity->getPosition()) && !entity->isDead())
        {
            printf("is obstructed by building %s \n", entity->getStats().getName());
            return true;
        }
    }
    for (Entity* entity : friendlyPlayer.getMobs())
    {
        if (entity->getStats().getMobType() == iEntityStats::MobType::Giant)
        {
            if(lineSquareIntersection(e->getPosition(), entity->getStats().getSize(), entity->getPosition()) && !entity->isDead())
            {
                printf("is obstructed by giant %s \n", entity->getStats().getName());
                return true;
            }
        }
    }

    //printf("seen by %s \n", e->getStats().getName());
    return false;
}

bool Mob::isHidden() const
{
    // Project 2: This is where you should put the logic for checking if a Rogue is
    // hidden or not.  It probably involves something related to calling Game::Get()
    // to get the Game, then calling getPlayer() on the game to get each player, then
    // going through all the entities on the players and... well, you can take it 
    // from there.  Once you've implemented this function, you can use it elsewhere to
    // change the Rogue's behavior, damage, etc.  It is also used in by the Graphics
    // to change the way the character renders (Rogues on the South team will render
    // as grayed our when hidden, ones on the North team won't render at all).

    Player& northPlayer = Game::get().getPlayer(true);
    Player& southPlayer = Game::get().getPlayer(false);

    std::vector<Entity*> gameEntities;

    // Get all the mobs and buildings of each player, and insert them into the Entities Vector
    gameEntities.insert(gameEntities.end(), northPlayer.getMobs().begin(), northPlayer.getMobs().end());
    gameEntities.insert(gameEntities.end(), southPlayer.getMobs().begin(), southPlayer.getMobs().end());
    gameEntities.insert(gameEntities.end(), northPlayer.getBuildings().begin(), northPlayer.getBuildings().end());
    gameEntities.insert(gameEntities.end(), southPlayer.getBuildings().begin(), southPlayer.getBuildings().end());



    // As a placeholder, just mark Rogues as always hidden.
    if (getStats().getMobType() == iEntityStats::MobType::Rogue)
    {
        for (Entity* entity : gameEntities)
        {
            Vec2 direction = entity->getPosition() - m_Pos;
            float distance = direction.length();


            if (m_bNorth != entity->isNorth() && !entity->isDead() && entity->getStats().getSightRadius() >= distance)
            {
                //printf("is obstructed: %d\n", isObstructedByGiantOrTower(entity, Game::get().getPlayer(m_bNorth)));
                return isObstructedByGiantOrTower(entity, Game::get().getPlayer(m_bNorth));
              
            }

        }
        return true;
    }

    return false;
}

void Mob::move(float deltaTSec)
{
    // Project 2: You'll likely need to do some work in this function to get the 
    // Rogue to move correctly (i.e. hide behind Giant or Tower, move with the Giant,
    // spring out when doing a sneak attack, etc).
    //   Of note, putting special case code for a single type of unit into the base
    // class function like this IS AN AWFUL IDEA, because it wouldn't scale as more
    // types of units are added.  We only have the one special unit type, so we can 
    // afford to be a lazy in this instance... but if this were production code, I 
    // would never do it this way!!
    //   TODO: Build a better system for encapsulating unit-specific logic, perhaps
    // by repurposing and expanding the EntityStats subclasses (which need some love
    // as well!)

    // If we have a target and it's on the same side of the river, we move towards it.
    //  Otherwise, we move toward the bridge.
    bool bMoveToTarget = false;
    if (!!m_pTarget)
    {    
        bool imTop = m_Pos.y < (GAME_GRID_HEIGHT / 2);
        bool otherTop = m_pTarget->getPosition().y < (GAME_GRID_HEIGHT / 2);

        if (imTop == otherTop)
        {
            bMoveToTarget = true;
        }
    }

    Vec2 destPos;
    if (bMoveToTarget)
    { 
        m_pWaypoint = NULL;
        destPos = m_pTarget->getPosition();
    }
    else
    {
        if (!m_pWaypoint)
        {
            m_pWaypoint = pickWaypoint();
        }
        destPos = m_pWaypoint ? *m_pWaypoint : m_Pos;
    }

    // Actually do the moving
    Vec2 moveVec = destPos - m_Pos;
    float distRemaining = moveVec.normalize();
    float moveDist = m_Stats.getSpeed() * deltaTSec;

    // if we're moving to m_pTarget, don't move into it
    if (bMoveToTarget)
    {
        assert(m_pTarget);
        distRemaining -= (m_Stats.getSize() + m_pTarget->getStats().getSize()) / 2.f;
        distRemaining = std::max(0.f, distRemaining);
    }

    if (moveDist <= distRemaining)
    {
        m_Pos += moveVec * moveDist;
    }
    else
    {
        m_Pos += moveVec * distRemaining;

        // if the destination was a waypoint, find the next one and continue movement
        if (m_pWaypoint)
        {
            m_pWaypoint = pickWaypoint();
            destPos = m_pWaypoint ? *m_pWaypoint : m_Pos;
            moveVec = destPos - m_Pos;
            moveVec.normalize();
            m_Pos += moveVec * distRemaining;
        }
    }

    // Project 1: This is where your collision code will be called from
    Mob* otherMob = checkCollision();
    if (otherMob) {
        processCollision(otherMob, deltaTSec);
    }
}

const Vec2* Mob::pickWaypoint()
{
    // Project 2:  You may need to make some adjustments here, so that Rogues will go
    // back to a friendly tower when they have nothing to attack or hide behind, rather 
    // than suicide-rushing the enemy tower (which they can't damage).
    //   Again, special-case code in a base class function bad.  Encapsulation good.


    float smallestDistSq = FLT_MAX;
    const Vec2* pClosest = NULL;

    for (const Vec2& pt : Game::get().getWaypoints())
    {
        // Filter out any waypoints that are behind (or barely in front of) us.
        // NOTE: (0, 0) is the top left corner of the screen
        float yOffset = pt.y - m_Pos.y;
        if ((m_bNorth && (yOffset < 1.f)) ||
            (!m_bNorth && (yOffset > -1.f)))
        {
            continue;
        }

        float distSq = m_Pos.distSqr(pt);
        if (distSq < smallestDistSq) {
            smallestDistSq = distSq;
            pClosest = &pt;
        }
    }



    return pClosest;
}

// Project 1: 
//  1) return a vector of mobs that we're colliding with
//  2) handle collision with towers & river 
Mob* Mob::checkCollision() 
{
    //for (const Mob* pOtherMob : Game::get().getMobs())
    //{
    //    if (this == pOtherMob) 
    //    {
    //        continue;
    //    }

    //    // Project 1: YOUR CODE CHECKING FOR A COLLISION GOES HERE
    //}
    return NULL;
}

void Mob::processCollision(Mob* otherMob, float deltaTSec) 
{
    // Project 1: YOUR COLLISION HANDLING CODE GOES HERE
}

