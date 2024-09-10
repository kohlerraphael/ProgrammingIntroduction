OUT = projet
CXX = g++
CXXFLAGS = -Wall -std=c++11 `pkg-config --cflags gtkmm-3.0`
LDLIBS = `pkg-config --libs gtkmm-3.0`
OFILES = tools.o player.o ball.o map.o simulation.o projet.o gui.o

all: $(OUT)

$(OUT): $(OFILES)
	$(CXX) $(CXXFLAGS) $(OFILES) -o $@ $(LDLIBS)

clean:
	@echo " *** DELETED .o AND EXECUTABLE FILES ***"
	@/bin/rm -f *.o *.x *.c~ *.h~ $(OUT)