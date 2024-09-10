/*
 *	\file	map.h
 *	\author	Robin Bonny (283196) & Raphael Kohler (284316)
 *	\date	Spring Semester 2019
 *	\brief	Project COM-112(a)
 */

#ifndef HEADER_MAP_H

#include <vector>

#include "tools.h"

#define HEADER_MAP_H

class Obstacle {
   private:
    Point indices;
    Point ctr;

   public:
    Obstacle(int line, int col, int nbCell);
    Point getIndices() const;
    Point getCtr() const;
    bool invalidIndices(int nbCell) const;
    bool superposition(Obstacle const& comparator) const;
};

bool findDuplicates(std::vector<Obstacle> const& obstacles);

#endif