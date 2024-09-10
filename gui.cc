/*
 *  \file   gui.cc
 *  \author Robin Bonny (283196) & Raphael Kohler (284316)
 *  \date   Spring Semester 2019
 *  \brief  Project COM-112(a)
 */

#include <cairomm/context.h>
#include <math.h>
#include <iostream>

#include "define.h"
#include "gui.h"
#include "simulation.h"

using namespace std;

#define ORIGIN 0, 0
#define BORDER 10
#define BLACK 0.0, 0.0, 0.0
#define WHITE 1.0, 1.0, 1.0
#define REDISH 1.0, 0.0, 0.0
#define BLUEISH 0.0, 0.5, 1.0
#define GREENISH 0.0, 1.0, 0.0
#define ORANGEISH 0.9, 0.6, 0.0
#define YELLOWISH 1.0, 1.0, 0.0
#define BROWNISH 0.5, 0.2, 0.0
#define INDICATOR_COEFF 0.3

// ---------- PROTOTYPES ----------

static void drawObstacles(const Cairo::RefPtr<Cairo::Context>& pic, SimData& state);
static void drawPlayers(const Cairo::RefPtr<Cairo::Context>& pic, SimData& state);
static void drawBalls(const Cairo::RefPtr<Cairo::Context>& pic, SimData& state);
static double calcCounterAngle(unsigned int count);

// ---------- DECLARATIONS ----------

int startGUI() {
    auto app = Gtk::Application::create();
    GUI window;
    return app->run(window);
}

int startGUI(string startFile) {
    auto app = Gtk::Application::create();
    GUI window;
    if (!prepareSimulation(startFile) || getSimData().getPlayers().size() <= 1) {
        window.invalidateGUI();
    } else {
        window.validateGUI();
    }
    return app->run(window);
}

void drawPlayers(const Cairo::RefPtr<Cairo::Context>& pic, SimData& state) {
    for (unsigned int i = 0; i < state.getPlayers().size(); i++) {
        pic->set_line_width(state.getPlayerRadius());
        switch (state.getPlrHits(i)) {
            case 4:  // Safe
                pic->set_source_rgb(GREENISH);
                break;
            case 3:  // Gettin' hot here
                pic->set_source_rgb(YELLOWISH);
                break;
            case 2:  // Pretty risky
                pic->set_source_rgb(ORANGEISH);
                break;
            case 1:  // Near death experience
                pic->set_source_rgb(REDISH);
                break;
            default:  // Case for unpredicted value
                pic->set_source_rgb(BLACK);
                break;
        }
        double tempX = state.getPlrCtr(i).getX();
        double tempY = state.getPlrCtr(i).getY();
        pic->move_to(tempX + state.getPlayerRadius() / 2 + DIM_MAX, DIM_MAX - tempY);
        pic->arc(tempX + DIM_MAX, DIM_MAX - tempY, state.getPlayerRadius() / 2, 0,
                 2 * M_PI);
        pic->stroke();

        double indicatorWidth = state.getPlayerRadius() * INDICATOR_COEFF;
        pic->set_line_width(indicatorWidth);
        pic->set_source_rgb(BLUEISH);
        pic->move_to(tempX + DIM_MAX,
                     DIM_MAX - tempY - state.getPlayerRadius() + indicatorWidth / 2);
        pic->arc(tempX + DIM_MAX, DIM_MAX - tempY,
                 state.getPlayerRadius() - indicatorWidth / 2, -M_PI / 2,
                 calcCounterAngle(state.getPlrCnt(i)));
        pic->stroke();
    }
}

void drawBalls(const Cairo::RefPtr<Cairo::Context>& pic, SimData& state) {
    pic->set_source_rgb(BLUEISH);
    pic->set_line_width(state.getBallRadius());
    for (unsigned int i = 0; i < state.getBalls().size(); i++) {
        double tempX = state.getBlCtr(i).getX();
        double tempY = state.getBlCtr(i).getY();
        pic->move_to(tempX + state.getBallRadius() / 2 + DIM_MAX, DIM_MAX - tempY);
        pic->arc(tempX + DIM_MAX, DIM_MAX - tempY, state.getBallRadius() / 2, 0,
                 2 * M_PI);
    }
    pic->stroke();
}

double calcCounterAngle(unsigned int count) {
    double stepAngle = 2 * M_PI / MAX_COUNT;
    double counterAngle = (stepAngle * count) - M_PI / 2;
    return counterAngle;
}

/*
 *	\brief	Draw the objects using their corresponding functions
 */
bool Drawing::on_draw(const Cairo::RefPtr<Cairo::Context>& pic) {
    Gtk::Allocation allocation = get_allocation();
    SimData& state = getSimData();
    pic->set_source_rgb(WHITE);
    pic->paint();
    pic->set_source_rgb(BLACK);
    pic->rectangle(ORIGIN, allocation.get_width(), allocation.get_height());
    pic->stroke();
    drawObstacles(pic, state);
    drawPlayers(pic, state);
    drawBalls(pic, state);
    return true;
}

void Drawing::refresh() {
    auto win = get_window();
    if (win) {
        Gdk::Rectangle r(0, 0, get_allocation().get_width(),
                         get_allocation().get_height());
        win->invalidate_rect(r, false);
    }
}

GUI::GUI()
    : windowBox(Gtk::ORIENTATION_VERTICAL, BORDER),
      buttonBox(Gtk::ORIENTATION_HORIZONTAL, BORDER),
      drawBox(Gtk::ORIENTATION_HORIZONTAL, BORDER),
      exitButton("Exit"),
      openButton("Open"),
      saveButton("Save"),
      startButton("Start"),
      stepButton("Step"),
      labelInfo("No game to run"),
      simRunning(false),
      disconnect(false),
      timeoutValue(DELTA_T * 1000),
      resetSim(true)

{
    set_title("Projet Dodgeball");
    set_border_width(BORDER);
    set_resizable(false);
    add(windowBox);

    windowBox.pack_start(buttonBox, false, false);
    windowBox.pack_start(drawBox, false, false);
    drawArea.set_size_request(SIDE, SIDE);

    buttonBox.pack_start(exitButton, false, false);
    buttonBox.pack_start(openButton, false, false);
    buttonBox.pack_start(saveButton, false, false);
    buttonBox.pack_start(startButton, false, false);
    buttonBox.pack_start(stepButton, false, false);
    buttonBox.pack_start(labelInfo, false, false);
    drawBox.pack_start(drawArea, false, false);

    exitButton.signal_clicked().connect(sigc::mem_fun(*this, &GUI::clickExit));
    openButton.signal_clicked().connect(sigc::mem_fun(*this, &GUI::clickOpen));
    saveButton.signal_clicked().connect(sigc::mem_fun(*this, &GUI::clickSave));
    startButton.signal_clicked().connect(sigc::mem_fun(*this, &GUI::clickStart));
    stepButton.signal_clicked().connect(sigc::mem_fun(*this, &GUI::clickStep));

    show_all_children();
}

/*
 *	\brief	Activates or deactivates the buttons 'Start/Stop' and 'Step' when no
 *          file is loaded
 */
void GUI::validateGUI() {
    labelInfo.set_label("Game ready to run");
    startButton.set_sensitive(true);
    stepButton.set_sensitive(true);
}
void GUI::invalidateGUI() {
    startButton.set_sensitive(false);
    stepButton.set_sensitive(false);
}

// ---------- SIGNAL HANDLERS ----------

void GUI::clickExit() {
    hide();
    exit(EXIT_SUCCESS);
}

void GUI::clickOpen() {
    Gtk::FileChooserDialog dialog("Select an input file",
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);
    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Open", Gtk::RESPONSE_OK);
    int result = dialog.run();
    switch (result) {
        case (Gtk::RESPONSE_OK): {
            string filename = dialog.get_filename();
            if (!prepareSimulation(filename) ||
                getSimData().getPlayers().size() <= 1) {
                labelInfo.set_label("No game to run");
                invalidateGUI();
            } else {
                startButton.set_label("Start");
                validateGUI();
            }
            break;
        }
        case (Gtk::RESPONSE_CANCEL): {
            break;
        }
        default: { break; }
    }
    resetSim = true;
}

void GUI::clickSave() {
    Gtk::FileChooserDialog dialog("Select the file location",
                                  Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);
    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Save", Gtk::RESPONSE_OK);
    int result = dialog.run();
    switch (result) {
        case (Gtk::RESPONSE_OK): {
            string filename = dialog.get_filename();
            saveFile(filename);
            break;
        }
        case (Gtk::RESPONSE_CANCEL): {
            break;
        }
        default: { break; }
    }
}

void GUI::clickStart() {
    if (!simRunning) {  // START ACTION
        startButton.set_label("Stop");
        Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::onTimeout),
                                       timeoutValue);
        simRunning = true;
    } else {  // STOP ACTION
        startButton.set_label("Start");
        disconnect = true;
        simRunning = false;
    }
}

void GUI::clickStep() {
    if (!simRunning) stepGUI();
}

bool GUI::onTimeout() {
    if (disconnect) {
        disconnect = false;
        return false;
    }
    if (stepGUI())
        return true;
    else
        return false;
}

bool GUI::stepGUI() {
    if (getSimData().getPlayers().size() <= 1) {
        simRunning = false;
        labelInfo.set_label("Game's over!");
        invalidateGUI();
        return false;
    }
    if (!stepSimulation(resetSim)) {
        simRunning = false;
        labelInfo.set_label("Cannot complete the game!");
        invalidateGUI();
        return false;
    }
    drawArea.refresh();
    resetSim = false;
    return true;
}

void drawObstacles(const Cairo::RefPtr<Cairo::Context>& pic, SimData& state) {
    pic->set_source_rgb(BROWNISH);
    pic->set_line_width(state.getObstacleDim());
    for (unsigned int i = 0; i < state.getObstacles().size(); i++) {
        double tempX = state.getObsCtr(i).getX();
        double tempY = state.getObsCtr(i).getY();
        pic->move_to(tempX - state.getObstacleDim() / 2 + DIM_MAX, DIM_MAX - tempY);
        pic->line_to(tempX + state.getObstacleDim() / 2 + DIM_MAX, DIM_MAX - tempY);
    }
    pic->stroke();
}