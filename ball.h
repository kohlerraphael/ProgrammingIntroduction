/*
 *	\file	ball.h
 *	\author	Robin Bonny (283196) & Raphael Kohler (284316)
 *	\date	Spring Semester 2019
 *	\brief	Project COM-112(a)
 */

#ifndef HEADER_BALL_H

#include <vector>

#include "tools.h"

#define HEADER_BALL_H

class Ball {
   private:
    Point ctr;
    double angle;  // Angle of ball movement
    int index;
    double scaleCoeff;  // SIDE / nbCell

   public:
    Ball(double x, double y, double a, int index, int nbCell);
    Ball(double x, double y, double a, int nbCell);
    Point getCtr() const;
    double getAngle() const;
    double getScaleCoeff() const;
    bool outOfBounds(bool output) const;
    bool inContact(Ball const& comparator, executionState ex) const;
    void moveBall(double distance);
};

bool ballCollision(std::vector<Ball> const& balls);
void moveBalls(std::vector<Ball>& balls);
void stepBalls(std::vector<Ball>& balls);

#endif