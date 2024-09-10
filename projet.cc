/*
 *	\file	project.cc
 *	\author	Robin Bonny (283196) & Raphael Kohler (284316)
 *	\date	Spring Semester 2019
 *	\brief	Project COM-112(a)
 */

#include <iostream>
#include <string>

#include "gui.h"
#include "simulation.h"

using namespace std;

enum argumentNumber {
    PROGRAM_PATH,
    SECOND_ARGUMENT,
    THIRD_ARGUMENT,
    FOURTH_ARGUMENT,
    MAX_ARGUMENTS
};

int main(int argc, char *argv[]) {
    string inputFile;
    string outputFile;

    if (argc == 1) {
        // E.g.     ./projet
        startGUI();

    } else if (argc == 2) {
        // E.g.     ./projet input.txt
        inputFile = argv[SECOND_ARGUMENT];
        startGUI(inputFile);

    } else if (argc == 3 && string(argv[SECOND_ARGUMENT]) == "Error") {
        // E.g.     ./projet Error input.txt
        inputFile = argv[THIRD_ARGUMENT];
        prepareSimulation(inputFile);

    } else if (argc == MAX_ARGUMENTS && string(argv[SECOND_ARGUMENT]) == "Step") {
        // E.g.     ./projet Step input.txt output.txt
        inputFile = argv[THIRD_ARGUMENT];
        outputFile = argv[FOURTH_ARGUMENT];
        stepSimulationCMD(inputFile, outputFile);

    } else {
        exit(EXIT_FAILURE);
    }

    return 0;
}