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
    if (isHiding())
    {
        m_ticksSinceHidden++;
    }
    else 
    {
        m_ticksSinceHidden = 0;
    }

    // Tick the entity first.  This will pick our target, and attack it if it's in range.
    Entity::tick(deltaTSec);

    // if our target isn't in range, move towards it.
    if (!targetInRange())
    {
        move(deltaTSec);
    }
}
// Checks if two lines intersect.
bool lineLineIntersect(Vec2 p1, Vec2 p2, Vec2 q1, Vec2 q2, Vec2& intersection)
{
    float x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y;
    float x3 = q1.x, y3 = q1.y, x4 = q2.x, y4 = q2.y;

    float denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
    if (denom == 0.f) {
        // lines are parallel
        return false;  
    }

    float t1 = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denom;
    float t2 = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denom;

    if (t1 >= 0.f && t1 <= 1.f && t2 >= 0.f && t2 <= 1.f) {
        intersection.x = x1 + t1 * (x2 - x1);
        intersection.y = y1 + t1 * (y2 - y1);
        return true;
    }
    else {
        // line segments do not intersect
        return false;  
    }
}


bool Mob::lineSquareIntersection(Vec2 start, float size, Vec2 obj_pos) const
{
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




bool Mob::isObstructedByGiantOrTower(Entity* e, Player& friendlyPlayer) const
{
    // For each friendly building check if the vector between the entity and mob is 
    // intersected by a building.
    for (Entity* entity : friendlyPlayer.getBuildings())
    {
        if(lineSquareIntersection(e->getPosition(), entity->getStats().getSize(), entity->getPosition()) && !entity->isDead())
        {
            return true;
        }
    }
    // For each friendly building check if the vector between the entity and mob is 
    // intersected by a giant.   
    for (Entity* entity : friendlyPlayer.getMobs())
    {
        if (entity->getStats().getMobType() == iEntityStats::MobType::Giant)
        {
            if(lineSquareIntersection(e->getPosition(), entity->getStats().getSize(), entity->getPosition()) && !entity->isDead())
            {
                return true;
            }
        }
    }

    return false;
}


bool Mob::isHiding() const
{
    // Gets all the entities in the game.
    Player& northPlayer = Game::get().getPlayer(true);
    Player& southPlayer = Game::get().getPlayer(false);

    std::vector<Entity*> gameEntities;

    // Get all the mobs and buildings of each player, and insert them into the Entities Vector
    gameEntities.insert(gameEntities.end(), northPlayer.getMobs().begin(), northPlayer.getMobs().end());
    gameEntities.insert(gameEntities.end(), southPlayer.getMobs().begin(), southPlayer.getMobs().end());
    gameEntities.insert(gameEntities.end(), northPlayer.getBuildings().begin(), northPlayer.getBuildings().end());
    gameEntities.insert(gameEntities.end(), southPlayer.getBuildings().begin(), southPlayer.getBuildings().end());


    // If the mob is a rogue.
    if (getStats().getMobType() == iEntityStats::MobType::Rogue)
    {
        // for each entity in game entities.
        for (Entity* entity : gameEntities)
        {
            Vec2 direction = entity->getPosition() - m_Pos;
            float distance = direction.length();

            // If the entity is from the opposing player, its not dead and the mob is within the entity's sight radius.
            if (m_bNorth != entity->isNorth() && !entity->isDead() && entity->getStats().getSightRadius() >= distance)
            {
                // Check if the mob is not obstructed by a giant or tower from the entity.
                if (!isObstructedByGiantOrTower(entity, Game::get().getPlayer(m_bNorth)))
                {
                    return false;
                }

            }

        }
        return true;
    }

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

    // A mob is hidden if it has been hiding for longer than 2 seconds. 
    return isHiding() && m_ticksSinceHidden >= getStats().timeToHide() / 0.05f;

    
}



bool Mob::isEnemyInSpringAttackRange()
{
    Vec2 destPos;
    Player& opposingPlayer = Game::get().getPlayer(!m_bNorth);

    float springRange = getStats().getSpringRange();
    float springRangeSq = springRange * springRange;

    // Checks whether the opposing player has a mob that is in
    // spring attack range for each mob.
    for (Entity* pEntity : opposingPlayer.getMobs())
    {
        assert(pEntity->isNorth() != isNorth());
        if (!pEntity->isDead())
        {

            float distSq = m_Pos.distSqr(pEntity->getPosition());

            if (distSq < springRangeSq)
            {
                return true;
            }
        }
    }
    return false;

}


Vec2 Mob::getHidingLocation(Entity* friendlyObject)
{
    // Get the opposing player and sight range.
    Player& opposingPlayer = Game::get().getPlayer(!m_bNorth);
    float closestDist = getStats().getSightRadius() + 0.1f;
    float closestDistSq = closestDist * closestDist;

    // Get the game entities of the opposing player.
    std::vector<Entity*> gameEntities;
    std::vector<Entity*> opposingEntitiesInSight;

    gameEntities.insert(gameEntities.end(), opposingPlayer.getMobs().begin(), opposingPlayer.getMobs().end());
    gameEntities.insert(gameEntities.end(), opposingPlayer.getBuildings().begin(), opposingPlayer.getBuildings().end());

    // For each opposing player's game entity.
    for (Entity* pEntity : gameEntities)
    {
        assert(pEntity->isNorth() != isNorth());
        if (!pEntity->isDead())
        {
            // If the entity is within sight of the rogue
            float distSq = m_Pos.distSqr(pEntity->getPosition());
            if (distSq < closestDistSq)
            {
                // If the rogue would be seen by the entity
                float enemySightSq = (pEntity->getStats().getSightRadius() + 0.1f) * (pEntity->getStats().getSightRadius() + 0.1f);
                if (distSq <= enemySightSq)
                {
                    // Add the entity to those that the rogue is hiding from
                    opposingEntitiesInSight.push_back(pEntity);
                }
                    
            }
            
        }
    }
    
    Vec2 destPos = friendlyObject->getPosition();

    // If the rogue doesn't see any entities and is hidden.
    if (opposingEntitiesInSight.size() <=  0 && isHidden())
    {
        // Approach the entity the rogue is hiding behind at the vector between the rogue and entity.
        // Within the hiding distance.
        Vec2 testVec =  m_Pos - friendlyObject->getPosition();
        testVec.normalize();
        destPos += testVec * ((friendlyObject->getStats().getSize() / 1.6f) + getStats().getHideDistance());
        

    }
    // If the rogue doesn't see any enemies and is not hidden.
    else if (opposingEntitiesInSight.size() <= 0)
    {
        // Hide behind the friendly entity.
        if (m_bNorth)
        {
            destPos.y -= (friendlyObject->getStats().getSize() / 2.f) + getStats().getHideDistance();
        }
        else
        {
            destPos.y += (friendlyObject->getStats().getSize() / 2.f) + getStats().getHideDistance();
        }
    }
    // Otherwise
    else
    {
        // Get the average vector of all the entities the rogue is hiding from.
        float sumX = 0.f;
        float sumY = 0.f;
        for (Entity* pEntity : opposingEntitiesInSight)
        {
            Vec2 newPos = friendlyObject->getPosition() - pEntity->getPosition();
            sumX += newPos.x;
            sumY += newPos.y;
        }

        Vec2 avgHidingVector = Vec2(sumX / (float) opposingEntitiesInSight.size(), sumY / (float) opposingEntitiesInSight.size());
        avgHidingVector.normalize();

        // Hide behind buildings at further distances to not hide in the building
        if (m_bFollowingBuilding)
        {
            avgHidingVector = avgHidingVector * ((friendlyObject->getStats().getSize() / 1.6f) + getStats().getHideDistance());

        } 
        else
        {
            avgHidingVector = avgHidingVector * ((friendlyObject->getStats().getSize() / 2.f) + getStats().getHideDistance());

        }

        destPos += avgHidingVector;
    }
    return destPos;

}


bool Mob::friendlyGiantPreferRange()
{
    Player& friendlyPlayer = Game::get().getPlayer(m_bNorth);
    float closestDist = getStats().perferGiantRange();
    float closestDistSq = closestDist * closestDist;

    // For each giant check if it is within the prefer giant range.
    for (Entity* pEntity : friendlyPlayer.getMobs())
    {
        assert(pEntity->isNorth() == isNorth());
        if (!pEntity->isDead())
        {
            if (pEntity->getStats().getMobType() == iEntityStats::MobType::Giant)
            {
                float distSq = m_Pos.distSqr(pEntity->getPosition());
                if (distSq < closestDistSq)
                {
                    // If it is, set the giant to the target, and sent the flags.
                    closestDistSq = distSq;
                    m_pTarget = pEntity;
                    m_eFriendlyGiant = pEntity;
                    m_pWaypoint = NULL;
                    m_bFollowingGiant = true;
                    return true;

                }
            }
        }
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

    // If we have a target and it's on the same side of the river, we move towards it.
    //  Otherwise, we move toward the bridge.
    Vec2 destPos;
    Player& friendlyPlayer = Game::get().getPlayer(m_bNorth);
    bool bMoveToTarget = false;
    bool hasTarget = false;


    float closestDist = getStats().getSightRadius();
    float closestDistSq = closestDist * closestDist;

    // If the mob is not hidden
    if (!isHidden())
    {
        // Flags for conditions the rogue has get reset.
        isInSpringAttackRange = false;
        m_bFollowingGiant = false;
        m_bFollowingBuilding = false;

        if (!!m_pTarget)
        {
            bool imTop = m_Pos.y < (GAME_GRID_HEIGHT / 2);
            bool otherTop = m_pTarget->getPosition().y < (GAME_GRID_HEIGHT / 2);

            if (imTop == otherTop)
            {
                bMoveToTarget = true;
            }
        }

        
        if (bMoveToTarget)
        {
            m_pWaypoint = NULL;
            destPos = m_pTarget->getPosition();
            hasTarget = true;
        }
        // If the mob type is a Rogue.
        else if (getStats().getMobType() == iEntityStats::MobType::Rogue)
        {
            // Find the closest friendly Giant.
            for (Entity* pEntity : friendlyPlayer.getMobs())
            {
                assert(pEntity->isNorth() == isNorth());
                if (!pEntity->isDead())
                {
                    if (pEntity->getStats().getMobType() == iEntityStats::MobType::Giant)
                    {
                        float distSq = m_Pos.distSqr(pEntity->getPosition());
                        if (distSq < closestDistSq)
                        {
                            closestDistSq = distSq;
                            destPos = pEntity->getPosition();
                            m_pTarget = pEntity;
                            m_eFriendlyGiant = pEntity;
                            hasTarget = true;
                            m_pWaypoint = NULL;
                            m_bFollowingGiant = true;

                            /*destPos = getHidingLocation(pEntity);*/
                        }
                    }
                }
            }
            // Follow the closest Giant.
            if (m_bFollowingGiant)
            {
                destPos = getHidingLocation(m_eFriendlyGiant);
            }
            // If the Rogue hasn't found a target yet
            if (!hasTarget)
            {
                // Find the Closest Building.
                float closestDistSq = INFINITY;
                for (Entity* pEntity : friendlyPlayer.getBuildings())
                {
                    assert(pEntity->isNorth() == isNorth());
                    if (!pEntity->isDead())
                    {
                        if (pEntity->getStats().getType() == iEntityStats::Building)
                        {
                            
                            float distSq = m_Pos.distSqr(pEntity->getPosition());
                            
                            if (distSq < closestDistSq)
                            {
                                closestDistSq = distSq;
                                destPos = pEntity->getPosition();
                                m_pTarget = pEntity;
                                m_eFriendlyBuilding = pEntity;
                                hasTarget = true;
                                m_pWaypoint = NULL;
                                m_bFollowingBuilding = true;
                                
                                
                            }
                        }
                    }
                }
                // Follow the closest building found.
                if (m_bFollowingBuilding)
                {
                    destPos = getHidingLocation(m_eFriendlyBuilding);
                }
            }

        }
        // If no target has been found, choose a waypoint.
        if (!hasTarget)
        {
            if (!m_pWaypoint)
            {
                m_pWaypoint = pickWaypoint();
            }
            destPos = m_pWaypoint ? *m_pWaypoint : m_Pos;


        }
    }
    // The mob is hiding. 
    else
    {
        // If the enemy is in Spring attack range,
        // set the Spring attack flag and attack the target.
        if (isEnemyInSpringAttackRange())
        {
            printf("Spring attack!\n");
            isInSpringAttackRange = true;
            bMoveToTarget = true;
            m_pWaypoint = NULL;
            destPos = m_pTarget->getPosition();
        }
        // Else if the rogue is following a giant.
        else if (m_bFollowingGiant)
        {
            // Get the hiding place around the giant.
            destPos = getHidingLocation(m_eFriendlyGiant);

        }
        // Else if the rogue is following a build.
        else if (m_bFollowingBuilding)
        {
            // If there is a giant in the range the rogue would prefer
            if (friendlyGiantPreferRange())
            {
                // Follow the giant.
                destPos = getHidingLocation(m_eFriendlyGiant);
            }
            // Hide behind the building.
            destPos = getHidingLocation(m_eFriendlyBuilding);
        }
        else
        {
            // If you are not hiding behind a building or a giant, find the closest giant
            for (Entity* pEntity : friendlyPlayer.getMobs())
            {
                assert(pEntity->isNorth() == isNorth());
                if (!pEntity->isDead())
                {
                    if (pEntity->getStats().getMobType() == iEntityStats::MobType::Giant)
                    {
                        float distSq = m_Pos.distSqr(pEntity->getPosition());
                        if (distSq < closestDistSq)
                        {
                            closestDistSq = distSq;
                            destPos = pEntity->getPosition();
                            m_pTarget = pEntity;
                            m_eFriendlyGiant = pEntity;
                            hasTarget = true;
                            m_pWaypoint = NULL;
                            m_bFollowingGiant = true;
                        }
                    }
                }
            }
            // If the mob finds the closest giant, follow it.
            if (m_bFollowingGiant)
            {
                destPos = getHidingLocation(m_eFriendlyGiant);
            }

            // If the mob doesn't find the giant, find the closest building.
            if (!hasTarget)
            {
                for (Entity* pEntity : friendlyPlayer.getBuildings())
                {
                    assert(pEntity->isNorth() == isNorth());
                    if (!pEntity->isDead())
                    {
                        if (pEntity->getStats().getType() == iEntityStats::Building)
                        {
                            float distSq = m_Pos.distSqr(pEntity->getPosition());
                            if (distSq < closestDistSq)
                            {
                                closestDistSq = distSq;
                                destPos = pEntity->getPosition();
                                m_pTarget = pEntity;
                                m_eFriendlyBuilding = pEntity;
                                hasTarget = true;
                                m_pWaypoint = NULL;
                                m_bFollowingBuilding = true;
                                
                                
                            }
                        }
                    }
                }
                // If the mob finds the building, 
                // hide behind the building.
                if (m_bFollowingBuilding)
                {
                    destPos = getHidingLocation(m_eFriendlyBuilding);
                }
            }

        }
        
        

    }



    // Actually do the moving
    Vec2 moveVec = destPos - m_Pos;
    float distRemaining = moveVec.normalize();
    float moveDist;
    if (isInSpringAttackRange)
    {
        moveDist = m_Stats.getSpringSpeed() * deltaTSec;
        
    }
    else
    {
        moveDist = m_Stats.getSpeed() * deltaTSec;
    }
    

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

        {
            // Filter out any waypoints that are behind (or barely in front of) us.
            // NOTE: (0, 0) is the top left corner of the screen
            // TODO: Giant Waypoint Bug that came with the code
            float yOffset = pt.y - m_Pos.y;
            if ((m_bNorth && (yOffset < 1.f)) ||
                (!m_bNorth && (yOffset > -1.f)) ||
                (pt.y > 25.5f) ||
                (pt.y < 5.f))
            {
                continue;
            }

            float distSq = m_Pos.distSqr(pt);
            if (distSq < smallestDistSq) {
                smallestDistSq = distSq;
                pClosest = &pt;
            }
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

