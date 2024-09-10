/*
 *	\file	map.cc
 *	\author	Robin Bonny (283196) & Raphael Kohler (284316)
 *	\date	Spring Semester 2019
 *	\brief	Project COM-112(a)
 */

#include <iostream>
#include <vector>

#include "define.h"
#include "error.h"
#include "map.h"

using namespace std;

#define LOWER_BOUND 0

/*
 * \brief	Initializes objects; matrix indices are transformed to the regular
 * coordinate system
 */
Obstacle::Obstacle(int line, int col, int nbCell) : indices(line, col), ctr() {
    ctr = Point(col, line).mtx2cart(DIM_MAX, SIDE, nbCell);
}

Point Obstacle::getIndices() const { return indices; }
Point Obstacle::getCtr() const { return ctr; }

bool Obstacle::invalidIndices(int nbCell) const {
    if (indices.getX() >= nbCell || indices.getX() < LOWER_BOUND) {
        cout << OBSTACLE_VALUE_INCORRECT(int(indices.getX())) << endl;
        return true;
    }
    if (indices.getY() >= nbCell || indices.getY() < LOWER_BOUND) {
        cout << OBSTACLE_VALUE_INCORRECT(int(indices.getY())) << endl;
        return true;
    }
    return false;
}

bool Obstacle::superposition(Obstacle const& comparator) const {
    if (indices.getX() == comparator.indices.getX() &&
        indices.getY() == comparator.indices.getY())
        return true;
    else
        return false;
}

bool findDuplicates(vector<Obstacle> const& obstacles) {
    for (unsigned int i = 0; i < obstacles.size(); i++) {
        for (unsigned int j = 0; j < i; j++) {
            if (obstacles[i].superposition(obstacles[j])) {
                double param1 = obstacles[i].getIndices().getX();
                double param2 = obstacles[i].getIndices().getY();
                cout << MULTI_OBSTACLE(int(param1), int(param2)) << endl;
                return true;
            }
        }
    }
    return false;
}