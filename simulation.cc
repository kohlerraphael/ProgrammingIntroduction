/*
 *	\file	simulation.cc
 *	\author	Robin Bonny (283196) & Raphael Kohler (284316)
 *	\date	Spring Semester 2019
 *	\brief	Project COM-112(a)
 */

#include <math.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "define.h"
#include "error.h"
#include "simulation.h"
#include "tools.h"

using namespace std;

#define FILE_EXTENSION_LENGTH 4

enum readState {
    CELL,
    PLAYER,
    PLAYER_DATA,
    OBSTACLE,
    OBSTACLE_DATA,
    BALL,
    BALL_DATA,
    END
};

static SimData state;

// ---------- PROTOTYPES ----------

static bool readFile(string inputFile);
static void readLine(string content, bool newFile);
static bool searchErrors();
static bool collisionCalc(vector<Player> const &players, vector<Ball> const &balls,
                          executionState ex);
static bool collisionCalc(Player const &player, Ball const &ball);
static bool collisionCalc(vector<Player> const &players,
                          vector<Obstacle> const &obstacles, executionState ex);
static bool collisionCalc(vector<Ball> const &balls, vector<Obstacle> const &obstacles,
                          executionState ex);
static bool collisionCalc(Ball const &ball, Obstacle const &obstacle);
static bool callAlgo(unsigned int nbInvalidVertices);
static void stepBallPlayer();
static void stepBallObs(bool &recalcMtx);
static void stepPlayers();

/*
 *	\brief	Detects if a collision occurs between a player/ball and an
 *          obstacle
 */
static bool detectCollision(double radius, Point const &objCtr,
                            Obstacle const &obstacle, executionState ex);

// ---------- DECLARATIONS ----------

SimData &getSimData() { return state; }

void SimData::setExeState(executionState es) { ex = es; }
executionState SimData::getExeState() const { return ex; }

/*
 *	\brief	Reset vectors for a new simulation
 */
void SimData::reset() {
    obstacles.clear();
    players.clear();
    balls.clear();
}

void SimData::initValues(int input) {
    nbCell = input;
    playerRadius = COEF_RAYON_JOUEUR * (SIDE / nbCell);
    ballRadius = COEF_RAYON_BALLE * (SIDE / nbCell);
    safetyMarginGame = COEF_MARGE_JEU * (SIDE / nbCell);
    safetyMarginInput = (COEF_MARGE_JEU / 2) * (SIDE / nbCell);
    obstacleDim = (SIDE / nbCell);
}

bool prepareSimulation(string inputFile) {
    state.setExeState(IO);
    state.reset();
    if (readFile(inputFile)) {
        if (searchErrors()) {
            state.reset();
            return false;
        } else {
            cout << FILE_READING_SUCCESS << endl;
            state.setExeState(GAME);
            return true;
        }
    } else {
        return false;
    }
}

/*
 *	\brief	Execute one simulation step and print the output directly to a file
 */
void stepSimulationCMD(string inputFile, string outputFile) {
    prepareSimulation(inputFile);
    stepSimulation(true);
    saveFile(outputFile);
}

bool readFile(string inputFile) {
    bool newFile = true;
    ifstream file(inputFile);
    string content;
    if (!file.fail()) {
        while (getline(file >> ws, content)) {
            if (content.front() == '#') continue;  // Remove comments
            readLine(content, newFile);
            newFile = false;
        }
        file.close();
        return true;
    } else {
        file.close();
        return false;
    }
}

void readLine(string content, bool newFile) {
    static int nbCell, nbPlayer, playerCollisions, playerCounter, nbObstacle, obsRow,
        obsCol, nbBall, counter = 0, readState = CELL;
    static double playerX = 0.0, playerY = 0.0, ballX = 0.0, ballY = 0.0,
                  ballAngle = 0.0;
    string rest;
    istringstream data(content);

    // Reset readState to first state when a new file is read
    if (newFile) readState = CELL;

    switch (readState) {
        case CELL:
            data >> nbCell;
            state.initValues(nbCell);
            readState = PLAYER;
            break;
        case PLAYER:
            data >> nbPlayer;
            state.reservePlayers(nbPlayer);
            if (nbPlayer > 0)
                readState = PLAYER_DATA;
            else
                readState = OBSTACLE;
            break;
        case PLAYER_DATA:
            data >> playerX >> playerY >> playerCollisions >> playerCounter;
            state.addPlayer(Player(playerX, playerY, playerCollisions, playerCounter,
                                   counter, nbCell));
            if (counter < nbPlayer - INDEX_SHIFT_ONE)
                counter++;
            else {
                readState = OBSTACLE;
                counter = 0;
            }
            break;
        case OBSTACLE:
            data >> nbObstacle;
            state.reserveObstacles(nbObstacle);
            if (nbObstacle > 0)
                readState = OBSTACLE_DATA;
            else
                readState = BALL;
            break;
        case OBSTACLE_DATA:
            data >> obsRow >> obsCol;
            state.addObstacle(Obstacle(obsRow, obsCol, nbCell));
            if (counter < nbObstacle - INDEX_SHIFT_ONE)
                counter++;
            else {
                readState = BALL;
                counter = 0;
            }
            break;
        case BALL:
            data >> nbBall;
            state.reserveBalls(nbBall);
            if (nbBall > 0)
                readState = BALL_DATA;
            else
                readState = END;
            break;
        case BALL_DATA:
            data >> ballX >> ballY >> ballAngle;
            state.addBall(Ball(ballX, ballY, ballAngle, counter, nbCell));
            if (counter < nbBall - INDEX_SHIFT_ONE)
                counter++;
            else {
                readState = END;
                counter = 0;
            }
            break;
        case END:
            break;
    }
}

/*
 *	\brief	Search any present errors after a file is read
 */
bool searchErrors() {
    for (unsigned int i = 0; i < state.getBalls().size(); i++) {
        if (state.getBalls()[i].outOfBounds(true)) return true;
    }
    for (unsigned int i = 0; i < state.getPlayers().size(); i++) {
        if (state.getPlayers()[i].outOfBounds()) return true;
    }
    for (unsigned int i = 0; i < state.getObstacles().size(); i++) {
        if (state.getObstacles()[i].invalidIndices(state.getNbCell())) return true;
    }
    if (findDuplicates(state.getObstacles()) || playerCollision(state.getPlayers()) ||
        ballCollision(state.getBalls()) ||
        collisionCalc(state.getPlayers(), state.getBalls(), IO) ||
        collisionCalc(state.getPlayers(), state.getObstacles(), IO) ||
        collisionCalc(state.getBalls(), state.getObstacles(), IO))
        return true;

    return false;
}

bool stepSimulation(bool reset) {
    static bool recalcMtx = true;
    detectTarget(state.getPlayers());
    moveBalls(state.getBalls());

    for (unsigned int i = 0; i < state.getPlayers().size(); i++) {
        for (unsigned int j = 0; j < i; j++) {
            state.getPlayers()[i].inContact(state.getPlayers()[j], GAME);
        }
        state.getPlayers()[i].increaseCount();
    }
    static vector<Point> invalidVertices;
    if (recalcMtx || reset) {  // Recalculate matrix for changed map
        static int firstCall = true;
        invalidVertices.clear();
        invalidVertices.reserve(state.getObstacles().size());
        for (unsigned int i = 0; i < state.getObstacles().size(); i++) {
            invalidVertices.emplace_back(
                Point(state.getObstacles()[i].getIndices().getX(),
                      state.getObstacles()[i].getIndices().getY()));
        }
        if (firstCall || reset) {  // Initialize matrix on first call of new file
            state.getMatrix().setMatrix(state.getNbCell());
            firstCall = false;
        }
        state.getMatrix().initEdges(invalidVertices);
        recalcMtx = false;
    }
    // Execute pathfinding algorithm
    if (!callAlgo(invalidVertices.size())) return false;

    // Move objects and check for collisions along the way
    stepBallPlayer();
    stepBallObs(recalcMtx);
    detectTarget(state.getPlayers());
    stepPlayers();
    stepBalls(state.getBalls());

    return true;
}

/*
 *	\brief	Check if the path from source to target is blocked by an obstacle
 */
bool inSight(Point source, Point target, double angle) {
    vector<Player> invisiblePlayer;
    invisiblePlayer.emplace_back(Player(source.getX(), source.getY()));
    while (invisiblePlayer[0].getCtr().getDist(target) > state.getPlayerRadius()) {
        if (collisionCalc(invisiblePlayer, state.getObstacles(), GAME))
            return false;
        else if (invisiblePlayer[0].outOfBounds())
            return false;
        else
            invisiblePlayer[0].movePlayer(angle, state.getPlayerRadius());
    }
    return true;
}

/*
 *	\brief	Shoot balls onto targets if counter of source player is full and handle
 *          direct collisions between players
 */
void shoot(Player &source) {
    if (source.getCnt() >= MAX_COUNT && source.getTouching() == false) {
        Point ballSrc = source.getCtr();
        ballSrc.move(source.getPlayerTargetAngle(),
                     state.getPlayerRadius() + state.getBallRadius() +
                         COEF_MARGE_JEU * SIDE / state.getNbCell());
        state.getBalls().emplace_back(Ball(ballSrc.getX(), ballSrc.getY(),
                                           M_PI - source.getPlayerTargetAngle(),
                                           state.getNbCell()));
        source.resetCount();
    } else if (source.getCnt() >= MAX_COUNT && source.getTouching() == true) {
        source.injure(source.getPlayerTarget(), state.getPlayers());
        source.resetCount();
    }
}

void saveFile(string outputFile) {
    if (outputFile.substr(outputFile.length() - FILE_EXTENSION_LENGTH) != ".txt")
        outputFile.append(".txt");
    ofstream file(outputFile);
    if (!file.fail()) {
        file << "# nbCell" << endl;
        file << state.getNbCell() << endl;
        file << endl << "# Players" << endl;
        file << state.getPlayers().size() << endl;
        for (unsigned int i = 0; i < state.getPlayers().size(); i++) {
            file << state.getPlayers()[i].getCtr().getX() << " "
                 << state.getPlayers()[i].getCtr().getY() << " "
                 << state.getPlayers()[i].getHits() << " "
                 << state.getPlayers()[i].getCnt() << endl;
        }
        file << endl << "# Obstacles" << endl;
        file << state.getObstacles().size() << endl;
        for (unsigned int i = 0; i < state.getObstacles().size(); i++) {
            file << state.getObstacles()[i].getIndices().getX() << " "
                 << state.getObstacles()[i].getIndices().getY() << endl;
        }
        file << endl << "# Balls" << endl;
        file << state.getBalls().size() << endl;
        for (unsigned int i = 0; i < state.getBalls().size(); i++) {
            file << state.getBalls()[i].getCtr().getX() << " "
                 << state.getBalls()[i].getCtr().getY() << " "
                 << state.getBalls()[i].getAngle() << endl;
        }
    } else
        exit(EXIT_FAILURE);
    file.close();
}

bool collisionCalc(vector<Player> const &players, vector<Ball> const &balls,
                   executionState ex) {
    for (unsigned int i = 0; i < players.size(); i++) {
        for (unsigned int j = 0; j < balls.size(); j++) {
            double dist = players[i].getCtr().getDist(balls[j].getCtr());
            if (ex == IO && dist < (state.getPlayerRadius() + state.getBallRadius() +
                                    state.getSftyMrgnIO())) {
                cout << PLAYER_BALL_COLLISION(i + INDEX_SHIFT_ONE, j + INDEX_SHIFT_ONE)
                     << endl;
                return true;
            } else if (ex == GAME &&
                       dist < (state.getPlayerRadius() + state.getBallRadius() +
                               state.getSftyMrgnG())) {
                return true;
            }
        }
    }
    return false;
}

bool collisionCalc(Player const &player, Ball const &ball) {
    double dist = player.getCtr().getDist(ball.getCtr());
    if (dist <
        (state.getPlayerRadius() + state.getBallRadius() + state.getSftyMrgnG()))
        return true;
    else
        return false;
}

bool collisionCalc(vector<Player> const &players, vector<Obstacle> const &obstacles,
                   executionState ex) {
    double radius;
    Point objCtr;
    for (unsigned int i = 0; i < players.size(); i++) {
        for (unsigned int j = 0; j < obstacles.size(); j++) {
            radius = state.getPlayerRadius();
            objCtr = players[i].getCtr();
            if (detectCollision(radius, objCtr, obstacles[j], ex)) {
                if (ex == IO) {
                    cout << COLL_OBST_PLAYER(j + INDEX_SHIFT_ONE, i + INDEX_SHIFT_ONE)
                         << endl;
                }
                return true;
            }
        }
    }
    return false;
}

bool collisionCalc(vector<Ball> const &balls, vector<Obstacle> const &obstacles,
                   executionState ex) {
    double radius;
    Point objCtr;
    for (unsigned int i = 0; i < balls.size(); i++) {
        for (unsigned int j = 0; j < obstacles.size(); j++) {
            radius = state.getBallRadius();
            objCtr = balls[i].getCtr();
            if (detectCollision(radius, objCtr, obstacles[j], ex)) {
                cout << COLL_BALL_OBSTACLE(i + INDEX_SHIFT_ONE) << endl;
                return true;
            }
        }
    }
    return false;
}

bool collisionCalc(Ball const &ball, Obstacle const &obstacle) {
    if (detectCollision(state.getBallRadius(), ball.getCtr(), obstacle, GAME))
        return true;
    else
        return false;
}

/*
 *	\brief	Call the pathfinding algorithm with all necessary data
 */
bool callAlgo(unsigned int nbInvalidVertices) {
    for (unsigned int i = 0; i < state.getPlayers().size(); i++) {
        Point source(state.getPlayers()[i].getCtr());
        Point target(state.getPlayers()[i].getPlayerTarget());
        source = source.cart2mtx(DIM_MAX, SIDE, state.getNbCell());
        target = target.cart2mtx(DIM_MAX, SIDE, state.getNbCell());
        Point movingTarget =
            state.getMatrix().pathFinder(source, target, nbInvalidVertices);
        if (movingTarget == INVALID_POINT) {
            return false;
        }
        state.getPlayers()[i].setMovingTarget(movingTarget);
    }
    return true;
}

/*
 *	\brief	Collide shot balls with players
 */
void stepBallPlayer() {
    for (unsigned int i = 0; i < state.getPlayers().size(); i++) {
        for (unsigned int j = 0; j < state.getBalls().size(); j++) {
            if (collisionCalc(state.getPlayers()[i], state.getBalls()[j])) {
                state.getPlayers()[i].injure(state.getPlayers()[i].getCtr(),
                                             state.getPlayers());
                state.getBalls().erase(state.getBalls().begin() + j);
            }
        }
    }
}

/*
 *	\brief  Collide shot balls with obstacles
 */
void stepBallObs(bool &recalcMtx) {
    for (unsigned int i = 0; i < state.getBalls().size(); i++) {
        for (unsigned int j = 0; j < state.getObstacles().size(); j++) {
            if (collisionCalc(state.getBalls()[i], state.getObstacles()[j])) {
                state.getBalls().erase(state.getBalls().begin() + i);
                state.getObstacles().erase(state.getObstacles().begin() + j);
                recalcMtx = true;
            }
        }
    }
}

/*
 *	\brief	Check if players are in direct sight and move correspondingly
 */
void stepPlayers() {
    for (unsigned int i = 0; i < state.getPlayers().size(); i++) {
        if (inSight(state.getPlayers()[i].getCtr(),
                    state.getPlayers()[i].getPlayerTarget(),
                    state.getPlayers()[i].getPlayerTargetAngle())) {
            state.getPlayers()[i].movePlayersDirect(state.getNbCell());
            shoot(state.getPlayers()[i]);
        } else {
            state.getPlayers()[i].movePlayersIndirect(state.getNbCell());
        }
    }
}

bool detectCollision(double radius, Point const &objCtr, Obstacle const &obstacle,
                     executionState ex) {
    // Ignore obstacles that are further away than the maximal collision distance
    double dist = objCtr.getDist(obstacle.getCtr());
    if (dist >
        (sqrt(2) * (state.getObstacleDim() / 2) + radius + state.getSftyMrgnIO())) {
        return false;
    }

    Point tempPoint(fabs(objCtr.getX() - obstacle.getCtr().getX()),
                    fabs(objCtr.getY() - obstacle.getCtr().getY()));
    // Find objects that collide with the obstacle  at the edge
    if (ex == IO && tempPoint.getX() < state.getObstacleDim() / 2 &&
        tempPoint.getY() < state.getObstacleDim() / 2 + radius + state.getSftyMrgnIO())
        return true;
    else if (ex == GAME && tempPoint.getX() < state.getObstacleDim() / 2 &&
             tempPoint.getY() <
                 state.getObstacleDim() / 2 + radius + state.getSftyMrgnG())
        return true;
    if (ex == IO && tempPoint.getY() < state.getObstacleDim() / 2 &&
        tempPoint.getX() < state.getObstacleDim() / 2 + radius + state.getSftyMrgnIO())
        return true;
    else if (ex == GAME && tempPoint.getY() < state.getObstacleDim() / 2 &&
             tempPoint.getX() <
                 state.getObstacleDim() / 2 + radius + state.getSftyMrgnG())
        return true;

    // Find objects that collide with the obstacle at corner
    Point corner(state.getObstacleDim() / 2, state.getObstacleDim() / 2);
    if (ex == IO && tempPoint.getDist(corner) < radius + state.getSftyMrgnIO())
        return true;
    else if (ex == GAME && tempPoint.getDist(corner) < radius + state.getSftyMrgnG())
        return true;
    else
        return false;
}

int SimData::getNbCell() const { return nbCell; }
double SimData::getPlayerRadius() const { return playerRadius; }
double SimData::getBallRadius() const { return ballRadius; }
double SimData::getSftyMrgnG() const { return safetyMarginGame; }
double SimData::getSftyMrgnIO() const { return safetyMarginInput; }
double SimData::getObstacleDim() const { return obstacleDim; }
void SimData::addPlayer(Player const &player) { players.emplace_back(player); }
void SimData::addObstacle(Obstacle const &obstacle) {
    obstacles.emplace_back(obstacle);
}
void SimData::addBall(Ball const &ball) { balls.emplace_back(ball); }
void SimData::reservePlayers(unsigned int size) { players.reserve(size); }
void SimData::reserveObstacles(unsigned int size) { obstacles.reserve(size); }
void SimData::reserveBalls(unsigned int size) { balls.reserve(size); }
vector<Player> &SimData::getPlayers() { return players; }
vector<Obstacle> &SimData::getObstacles() { return obstacles; }
vector<Ball> &SimData::getBalls() { return balls; }
Point SimData::getObsCtr(unsigned int index) const {
    return obstacles[index].getCtr();
}
Point SimData::getPlrCtr(unsigned int index) const { return players[index].getCtr(); }
Point SimData::getBlCtr(unsigned int index) const { return balls[index].getCtr(); }
int SimData::getPlrHits(unsigned int index) const { return players[index].getHits(); }
int SimData::getPlrCnt(unsigned int index) const { return players[index].getCnt(); }
Graph &SimData::getMatrix() { return adjMatrix; }
