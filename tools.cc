/*
 *	\file	tools.cc
 *	\author	Robin Bonny (283196) & Raphael Kohler (284316)
 *	\date	Spring Semester 2019
 *	\brief	Project COM-112(a)
 */

#include <math.h>
#include <iostream>
#include <vector>

#include "tools.h"

using namespace std;

#define MAX_EDGES (nbEdges + 1)
#define PATH_MIN_DIST 2
#define DIRECT_PATH_INDEX 1

Point::Point(double x, double y) : xCoord(x), yCoord(y) {}

void Point::setPoint(double x, double y) {
    xCoord = x;
    yCoord = y;
}

double Point::getX() const { return xCoord; }
double Point::getY() const { return yCoord; }

double Point::getDist(Point const &other) const {
    return sqrt(pow((xCoord - other.xCoord), 2) + pow((yCoord - other.yCoord), 2));
}

void Point::move(double angle, double distance) {
    xCoord += sin(angle) * distance;
    yCoord += cos(angle) * distance;
}

/*
 *	\brief	Transforms coordinates from matrix indices to the corresponding
 *          Cartesian coordinates
 */
Point Point::mtx2cart(int maxDim, int side, int cells) {
    double cartX = getX() * side / cells - maxDim + (side / (2 * cells));
    double cartY = (cells - INDEX_SHIFT_ONE - getY()) * side / cells - maxDim +
                   (side / (2 * cells));
    return Point(cartX, cartY);
}

/*
 *	\brief	Transforms coordinates from Cartesin coordinates to the corresponding
 *          matrix indices
 */
Point Point::cart2mtx(int maxDim, int side, int cells) {
    double mtxY = ((-1) * (side - 2 * cells * (getX() + maxDim)) / (2 * side)) + 0.5;
    double mtxX =
        (((2 * cells - INDEX_SHIFT_ONE) * side - 2 * cells * (getY() + maxDim)) /
         (2 * side)) +
        0.5;
    return Point((unsigned int)(mtxX), (unsigned int)(mtxY));
}

bool Point::operator==(Point other) {
    if (xCoord == other.xCoord && yCoord == other.yCoord)
        return true;
    else
        return false;
}
bool Point::operator!=(Point other) { return (!((*this) == other)); }

/*
 *	\brief	Initializes the matrix as a 2D vector of appropriate dimensions and
 *          fills the vector of vertices
 */
void Graph::setMatrix(unsigned int dim) {
    vertices.clear();
    dimension = pow(dim, 2);
    nbEdges = 0;
    matrix.resize(dimension);
    for (unsigned int i = 0; i < dimension; i++) {
        matrix[i].resize(dimension);
    }
    for (unsigned int i = 0; i < dimension; i++) {
        for (unsigned int j = 0; j < dimension; j++) {
            matrix[i][j] = 0.0;
        }
    }
    // Reserve space for the maximum number of vertices
    vertices.reserve(2 * (dimension - 1) * (2 * dimension - 1));
    for (unsigned int i = 0; i < int(sqrt(dimension)); i++) {
        for (unsigned int j = 0; j < int(sqrt(dimension)); j++) {
            vertices.emplace_back(Vertex(Point(i, j)));
        }
    }
}

/*
 *	\brief	Fills the values of the initialized adjacency matrix with data of the
 *          edges and their weight present in the graph
 */
void Graph::initEdges(vector<Point> const &invalidVertices) {
    for (unsigned int i = 0; i < dimension; i++) {
        for (unsigned int j = 0; j <= i; j++) {
            // HORIZONTAL AND VERTICAL CONNECTIONS
            if ((i % int(sqrt(dimension)) != 0 && abs(int(i - j)) == 1) ||
                abs(int(i - j)) == sqrt(dimension)) {
                addEdge(i, j, 1);
            }
            // DIAGONAL CONNECTIONS
            if (((i + 1) % int(sqrt(dimension)) != 0 &&
                 abs(int(i - j)) == int(sqrt(dimension)) - 1) ||
                ((i + int(sqrt(dimension))) % int(sqrt(dimension)) != 0 &&
                 abs(int(i - j)) == int(sqrt(dimension)) + 1)) {
                addEdge(i, j, sqrt(2));
            }
            if (i == j) {
                matrix[i][j] = false;
            }
        }
    }
    for (unsigned int i = 0; i < invalidVertices.size(); i++) {
        int invalidIndex = invalidVertices[i].getX() * int(sqrt(dimension)) +
                           invalidVertices[i].getY();

        for (int i = 0; i < 2; i++) {
            // DIRECT CONNECTIONS
            removeEdge(invalidIndex, invalidIndex + pow(-1, i));
            removeEdge(invalidIndex, invalidIndex + pow(-1, i) * sqrt(dimension));
            removeEdge(invalidIndex, invalidIndex + sqrt(dimension) + pow(-1, i));
            removeEdge(invalidIndex, invalidIndex - sqrt(dimension) + pow(-1, i));
            // TANGENTIAL CONNECTIONS
            removeEdge(invalidIndex + pow(-1, i), invalidIndex + sqrt(dimension));
            removeEdge(invalidIndex + pow(-1, i), invalidIndex - sqrt(dimension));
        }
    }
}

/*
 *	\brief	Traverse the prepared graph using Dijkstra's algorithm
 */
Point Graph::pathFinder(Point source, Point target, unsigned int inacessibleVertices) {
    vector<Vertex *> shortestPath;
    vector<Vertex> distance = vertices;
    for (unsigned int i = 0; i < distance.size(); i++) {
        if (distance[i].getPos() == source)
            distance[i].setDist(0);
        else
            distance[i].setDist(MAX_EDGES);
    }
    static bool stuckLoop = false;
    // While not all vertices have been visited
    while (shortestPath.size() < distance.size() - inacessibleVertices) {
        if (!stuckLoop) {
            stuckLoop = true;
            for (unsigned int i = 0; i < distance.size(); i++) {
                // Find next unvisited vertex
                if (distance[i].getVisited() == false &&
                    distance[i].getDist() < MAX_EDGES) {
                    shortestPath.emplace_back(&distance[i]);
                    distance[i].visit();
                    iterateAdjacentVertices(i, distance);
                    stuckLoop = false;
                }
            }
        } else {
            stuckLoop = false;
            return INVALID_POINT;
        }
    }
    Vertex trgtVertex;
    for (unsigned int i = 0; i < distance.size(); i++) {
        if (distance[i].getPos() == target) {
            trgtVertex = distance[i];
            break;
        }
    }
    return trgtVertex.backtrackPoint(distance, true);
}

vector<Vertex *> Graph::getAdjVertices(unsigned int currentVertex,
                                       vector<Vertex> &distance) const {
    vector<Vertex *> adjacentVertices;
    for (unsigned int adjVertex = 0; adjVertex < dimension; adjVertex++) {
        // Find adjacent vertices from matrix to unvisited vertex
        if (matrix[currentVertex][adjVertex] != 0.0) {
            adjacentVertices.emplace_back(&distance[adjVertex]);
        }
    }
    return adjacentVertices;
}

void Graph::iterateAdjacentVertices(unsigned int currentVertex,
                                    std::vector<Vertex> &distance) {
    vector<Vertex *> adjVertices = getAdjVertices(currentVertex, distance);
    for (unsigned int i = 0; i < adjVertices.size(); i++) {
        unsigned int adjVertex =
            adjVertices[i]->getPos().getX() * int(sqrt(dimension)) +
            adjVertices[i]->getPos().getY();
        if (distance[currentVertex].getDist() + matrix[currentVertex][adjVertex] <
            distance[adjVertex].getDist()) {
            distance[adjVertex].setDist(distance[currentVertex].getDist() +
                                        matrix[currentVertex][adjVertex]);
            distance[adjVertex].setPathInd(currentVertex);
        }
    }
}

/*
 *	\brief	Insert or remove an edge of given weight into the adjacency matrix
 */
void Graph::addEdge(unsigned int origin, unsigned int destination, double distance) {
    if (origin >= 0 && destination >= 0) {
        if (origin < dimension && destination < dimension) {
            matrix[origin][destination] = distance;
            matrix[destination][origin] = distance;
            nbEdges++;
        }
    }
}
void Graph::removeEdge(unsigned int origin, unsigned int destination) {
    if (origin >= 0 && destination >= 0) {
        if (origin < dimension && destination < dimension) {
            if (matrix[origin][destination] != 0.0 &&
                matrix[destination][origin] != 0.0) {
                matrix[origin][destination] = 0;
                matrix[destination][origin] = 0;
                nbEdges--;
            }
        }
    }
}

Vertex::Vertex(Point coordinates) : position(coordinates), visited(false) {}
void Vertex::visit() { visited = true; }
Point Vertex::getPos() const { return position; }
bool Vertex::getVisited() const { return visited; }
void Vertex::setDist(double distance) { distFromSrc = distance; }
double Vertex::getDist() const { return distFromSrc; }
void Vertex::setPathInd(unsigned int index) { pathIndex = index; }
unsigned int Vertex::getPathInd() const { return pathIndex; }

/*
 *	\brief  Get the path from one point to another by backtracking recursively from
 *          target to source
 */
Point Vertex::backtrackPoint(vector<Vertex> &distance, bool reset = false) {
    static vector<Vertex *> directions;
    static bool terminate = false;
    if (reset) {
        directions.resize(0);
        terminate = false;
    }
    while (distFromSrc > 0 && terminate == false) {
        directions.emplace_back(&distance[getPathInd()]);
        distance[getPathInd()].backtrackPoint(distance);
        if (distFromSrc < PATH_MIN_DIST) terminate = true;
    }
    if (directions.size() > PATH_MIN_DIST)
        return directions[directions.size() - PATH_MIN_DIST]->getPos();
    else if (directions.size() > DIRECT_PATH_INDEX)
        return directions[0]->getPos();
    else
        return getPos();
}