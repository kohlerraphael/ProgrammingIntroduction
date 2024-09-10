/*
 *	\file	player.cc
 *	\author	Robin Bonny (283196) & Raphael Kohler (284316)
 *	\date	Spring Semester 2019
 *	\brief	Project COM-112(a)
 */

#include <math.h>
#include <iostream>
#include <vector>

#include "define.h"
#include "error.h"
#include "player.h"

using namespace std;

Player::Player(double x, double y, int n, int c, int i, int nbCell)
    : ctr(x, y), hits(n), count(c), index(i), touching(false) {
    scaleCoeff = SIDE / nbCell;
}
Player::Player(double x, double y) : ctr(x, y) {}

Point Player::getCtr() const { return ctr; }
int Player::getHits() const { return hits; }
void Player::reduceLives() { hits--; }

/*
 *	\brief	Remove one life from the targeted player and delete the player if no
 *          lives are left
 */
void Player::injure(Point target, vector<Player>& players) {
    for (unsigned int i = 0; i < players.size(); i++) {
        if (target == players[i].getCtr()) {
            players[i].reduceLives();
            if (players[i].getHits() <= 0) {
                setTouching(false);
                players.erase(players.begin() + i);
            }
            break;
        }
    }
}

int Player::getCnt() const { return count; }
double Player::getScaleCoeff() const { return scaleCoeff; }
void Player::setPlayerTarget(Point const& targetPlayer) {
    playerTarget = targetPlayer;
}
Point Player::getPlayerTarget() const { return playerTarget; }
void Player::setPlayerTargetAngle(double angle) { playerTargetAngle = angle; }
double Player::getPlayerTargetAngle() const { return playerTargetAngle; }
void Player::setMovingTarget(Point const& targetVertex) {
    movingTarget = targetVertex;
}
Point Player::getMovingTarget() const { return movingTarget; }
bool Player::getTouching() const { return touching; }
void Player::setTouching(bool touch) { touching = touch; }
void Player::increaseCount() {
    if (count < MAX_COUNT) count++;
}
void Player::resetCount() { count = 0; }

bool Player::outOfBounds() const {
    if (fabs(ctr.getX()) > DIM_MAX || fabs(ctr.getY()) > DIM_MAX) {
        cout << PLAYER_OUT(index + INDEX_SHIFT_ONE) << endl;
        return true;
    } else
        return false;
}

bool Player::inContact(Player& comparator, executionState ex) {
    double dist = ctr.getDist(comparator.ctr);
    if (ex == IO &&
        dist < (scaleCoeff * (2 * COEF_RAYON_JOUEUR + COEF_MARGE_JEU / 2))) {
        touching = true;
        comparator.touching = true;
        return true;
    } else if (ex == GAME &&
               dist < (scaleCoeff * (2 * COEF_RAYON_JOUEUR + COEF_MARGE_JEU / 2))) {
        touching = true;
        comparator.touching = true;
        return true;
    } else {
        return false;
    }
}

void Player::movePlayer(double angle, double distance) {
    if (touching == false) {
        ctr.move(angle, distance);
    }
}

/*
 *	\brief	Move players directly to their target
 */
void Player::movePlayersDirect(int nbCell) {
    Point target = getPlayerTarget();
    Point source = getCtr();
    double secX = target.getX() - source.getX();
    double secY = target.getY() - source.getY();
    double angle = atan2(secX, secY);
    movePlayer(angle, COEF_VITESSE_JOUEUR * getScaleCoeff() * DELTA_T);
}

/*
 *	\brief	Move players indirectly to their target using the applied algorithm
 */
void Player::movePlayersIndirect(int nbCell) {
    Point target = getMovingTarget().mtx2cart(DIM_MAX, SIDE, nbCell);
    Point source = getCtr();
    double secX = (-1) * (target.getY() + source.getX());
    double secY = (-1) * (target.getX() + source.getY());
    double angle = atan2(secX, secY);
    movePlayer(angle, COEF_VITESSE_JOUEUR * getScaleCoeff() * DELTA_T);
}

bool playerCollision(vector<Player>& players) {
    for (unsigned int i = 0; i < players.size(); i++) {
        for (unsigned int j = 0; j < i; j++) {
            if (players[i].inContact(players[j], IO)) {
                cout << PLAYER_COLLISION(j + INDEX_SHIFT_ONE, i + INDEX_SHIFT_ONE)
                     << endl;
                return true;
            }
        }
    }
    return false;
}

/*
 *	\brief	Find closest player and set the target and angle attributes
 *          appropriately
 */
void detectTarget(vector<Player>& players) {
    unsigned int targetPlayer;
    for (unsigned int i = 0; i < players.size(); i++) {
        // Sufficiently large value for players to choose correct target
        double maxDist = pow(DIM_MAX, 2);
        targetPlayer = 0;
        for (unsigned int j = 0; j < players.size(); j++) {
            if (i != j) {
                double currentDist = players[i].getCtr().getDist(players[j].getCtr());
                if (currentDist < maxDist) {
                    maxDist = currentDist;
                    targetPlayer = j;
                }
            }
        }
        double secX =
            players[targetPlayer].getCtr().getX() - players[i].getCtr().getX();
        double secY =
            players[targetPlayer].getCtr().getY() - players[i].getCtr().getY();
        double angle = atan2(secX, secY);
        players[i].setPlayerTarget(players[targetPlayer].getCtr());
        players[i].setPlayerTargetAngle(angle);
    }
}
