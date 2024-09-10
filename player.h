/*
 *	\file	player.h
 *	\author	Robin Bonny (283196) & Raphael Kohler (284316)
 *	\date	Spring Semester 2019
 *	\brief	Project COM-112(a)
 */

#ifndef HEADER_PLAYER_H

#include <vector>

#include "tools.h"

#define HEADER_PLAYER_H

class Player {
   private:
    Point ctr;
    int hits;
    int count;
    int index;
    double scaleCoeff;  // SIDE / nbCell
    Point playerTarget;
    double playerTargetAngle;
    Point movingTarget;
    bool touching;

   public:
    Player(double x, double y, int n, int c, int i, int nbCell);
    Player(double x, double y);
    Point getCtr() const;
    int getHits() const;
    void reduceLives();
    void injure(Point target, std::vector<Player>& players);
    int getCnt() const;
    double getScaleCoeff() const;
    void setPlayerTarget(Point const& targetPlayer);
    Point getPlayerTarget() const;
    void setPlayerTargetAngle(double angle);
    double getPlayerTargetAngle() const;
    void setMovingTarget(Point const& targetVertex);
    Point getMovingTarget() const;
    bool getTouching() const;
    void setTouching(bool touch);
    void increaseCount();
    void resetCount();
    bool outOfBounds() const;
    bool inContact(Player& comparator, executionState ex);
    void movePlayer(double angle, double distance);
    void movePlayersDirect(int nbCell);
    void movePlayersIndirect(int nbCell);
};

bool playerCollision(std::vector<Player>& players);
void detectTarget(std::vector<Player>& players);

#endif