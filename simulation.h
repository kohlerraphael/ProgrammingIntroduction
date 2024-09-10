/*
 *	\file	simulation.h
 *	\author	Robin Bonny (283196) & Raphael Kohler (284316)
 *	\date	Spring Semester 2019
 *	\brief	Project COM-112(a)
 */

#ifndef HEADER_SIMULATION_H

#include <vector>

#include "ball.h"
#include "map.h"
#include "player.h"
#include "tools.h"

#define HEADER_SIMULATION_H

/*
 *	\brief	This class contains all constants needed for the simulation
 */
class SimData {
   private:
    std::vector<Player> players;
    std::vector<Obstacle> obstacles;
    std::vector<Ball> balls;

    executionState ex;
    int nbCell;
    double playerRadius;
    double ballRadius;
    double safetyMarginGame;
    double safetyMarginInput;
    double obstacleDim;

    Graph adjMatrix;

   public:
    void setExeState(executionState es);
    executionState getExeState() const;
    void reset();
    void addPlayer(Player const& player);
    void addObstacle(Obstacle const& obstacle);
    void addBall(Ball const& ball);
    void reservePlayers(unsigned int size);
    void reserveObstacles(unsigned int size);
    void reserveBalls(unsigned int size);

    std::vector<Player>& getPlayers();
    std::vector<Obstacle>& getObstacles();
    std::vector<Ball>& getBalls();

    Point getObsCtr(unsigned int index) const;
    Point getPlrCtr(unsigned int index) const;
    Point getBlCtr(unsigned int index) const;
    int getPlrHits(unsigned int index) const;
    int getPlrCnt(unsigned int index) const;

    void initValues(int input);

    int getNbCell() const;
    double getPlayerRadius() const;
    double getBallRadius() const;
    double getSftyMrgnG() const;
    double getSftyMrgnIO() const;
    double getObstacleDim() const;

    Graph& getMatrix();
};

bool prepareSimulation(std::string inputFile);
void stepSimulationCMD(std::string inputFile, std::string outputFile);
bool stepSimulation(bool reset);

void checkSights();
bool inSight(Point source, Point target, double angle);
void shoot(Player& source);

void saveFile(std::string outputFile);
SimData& getSimData();

#endif