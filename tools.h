/*
 *	\file	tools.h
 *	\author	Robin Bonny (283196) & Raphael Kohler (284316)
 *	\date	Spring Semester 2019
 *	\brief	Project COM-112(a)
 */

#ifndef HEADER_TOOLS_H
#define HEADER_TOOLS_H

#define INDEX_SHIFT_ONE 1
#define INVALID_POINT Point(-1, -1)

enum executionState { IO, GAME };

class Point {
   private:
    double xCoord;
    double yCoord;

   public:
    Point() = default;
    Point(double x, double y);
    void setPoint(double x, double y);
    double getX() const;
    double getY() const;
    double getDist(Point const& other) const;
    void move(double angle, double distance);
    Point mtx2cart(int maxDim, int side, int cells);
    Point cart2mtx(int maxDim, int side, int cells);
    bool operator==(Point other);
    bool operator!=(Point other);
};

class Vertex {
   private:
    Point position;
    bool visited;
    double distFromSrc;
    unsigned int pathIndex;

   public:
    Vertex() = default;
    Vertex(Point coordinates);
    void visit();

    Point getPos() const;
    bool getVisited() const;
    void setDist(double distance);
    double getDist() const;
    void setPathInd(unsigned int index);
    unsigned int getPathInd() const;
    Point backtrackPoint(std::vector<Vertex>& distance, bool reset);
};

class Graph {
   private:
    unsigned int dimension;
    std::vector<std::vector<double>> matrix;
    std::vector<Vertex> vertices;
    unsigned int nbEdges;

   public:
    void setMatrix(unsigned int dim);
    void resetMatrix();
    void initEdges(std::vector<Point> const& invalidVertices);
    unsigned int getNbVertices() const;
    Point pathFinder(Point source, Point target, unsigned int inacessibleVertices);
    std::vector<Vertex*> getAdjVertices(unsigned int currentVertex,
                                        std::vector<Vertex>& distance) const;
    void iterateAdjacentVertices(unsigned int currentVertex,
                                 std::vector<Vertex>& distance);
    void addEdge(unsigned int origin, unsigned int destination, double distance);
    void removeEdge(unsigned int origin, unsigned int destination);
};

#endif