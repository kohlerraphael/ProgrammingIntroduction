/*
 *	\file	gui.h
 *	\author	Robin Bonny (283196) & Raphael Kohler (284316)
 *	\date	Spring Semester 2019
 *	\brief	Project COM-112(a)
 */

#ifndef HEADER_GUI_H

#include <gtkmm.h>
#include <string>

#include "simulation.h"

#define HEADER_GUI_H

int startGUI();
int startGUI(std::string startFile);

class Drawing : public Gtk::DrawingArea {
   private:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& pic) override;

   public:
    void refresh();
};

class GUI : public Gtk::Window {
   private:
    void clickExit();
    void clickOpen();
    void clickSave();
    void clickStart();
    void clickStep();
    bool onTimeout();
    bool stepGUI();

    Drawing drawArea;
    Gtk::Box windowBox, buttonBox, drawBox;
    Gtk::Button exitButton, openButton, saveButton, startButton, stepButton;
    Gtk::Label labelInfo;

    bool simRunning;
    bool disconnect;
    const int timeoutValue;
    bool resetSim;

   public:
    GUI();
    void validateGUI();
    void invalidateGUI();
};

#endif