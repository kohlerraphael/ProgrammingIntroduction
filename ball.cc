/*
 *	\file	ball.cc
 *	\author	Robin Bonny (283196) & Raphael Kohler (284316)
 *	\date	Spring Semester 2019
 *	\brief	Project COM-112(a)
 */

#include <math.h>
#include <iostream>

#include "ball.h"
#include "define.h"
#include "error.h"

using namespace std;

Ball::Ball(double x, double y, double a, int i, int nbCell)
    : ctr(x, y), angle(a), index(i) {
    scaleCoeff = SIDE / nbCell;
}

Ball::Ball(double x, double y, double a, int nbCell) : ctr(x, y), angle(a) {
    scaleCoeff = SIDE / nbCell;
}

Point Ball::getCtr() const { return ctr; }
double Ball::getAngle() const { return angle; }
double Ball::getScaleCoeff() const { return scaleCoeff; }

bool Ball::outOfBounds(bool output) const {
    if (fabs(ctr.getX()) > DIM_MAX || fabs(ctr.getY()) > DIM_MAX) {
        if (output) cout << BALL_OUT(index + INDEX_SHIFT_ONE) << endl;
        return true;
    } else {
        return false;
    }
}

bool Ball::inContact(Ball const& comparator, executionState ex) const {
    double dist = ctr.getDist(comparator.ctr);
    if (ex == IO && dist < (scaleCoeff * (2 * COEF_RAYON_BALLE + COEF_MARGE_JEU / 2)))
        return true;
    else if (ex == GAME &&
             dist < (scaleCoeff * (2 * COEF_RAYON_BALLE + COEF_MARGE_JEU)))
        return true;
    else
        return false;
}

void Ball::moveBall(double distance) { ctr.move(M_PI - angle, distance); }

bool ballCollision(vector<Ball> const& balls) {
    for (unsigned int i = 0; i < balls.size(); i++) {
        for (unsigned int j = 0; j < i; j++) {
            if (balls[i].inContact(balls[j], IO)) {
                cout << BALL_COLLISION(j + INDEX_SHIFT_ONE, i + INDEX_SHIFT_ONE)
                     << endl;
                return true;
            }
        }
    }
    return false;
}

void moveBalls(vector<Ball>& balls) {
    for (unsigned int i = 0; i < balls.size(); i++) {
        balls[i].moveBall(COEF_VITESSE_BALLE * balls[i].getScaleCoeff() * DELTA_T);
        if (balls[i].outOfBounds(false)) balls.erase(balls.begin() + i);
    }
}

/*
 * \brief	For each simulation step determine if shot balls are in contact with
 *          each other and remove them
 */
void stepBalls(vector<Ball>& balls) {
    for (unsigned int i = 0; i < balls.size(); i++) {
        for (unsigned int j = 0; j < i; j++) {
            if (balls[i].inContact(balls[j], GAME)) {
                balls.erase(balls.begin() + i);
                balls.erase(balls.begin() + j);
            }
        }
    }
}