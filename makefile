sysname = $(shell uname -s)

ifeq ($(sysname), Linux)
  CXX = g++-10
  CXXFLAGS = -Ofast -ffast-math --std=c++17 -march=sandybridge -mtune=sandybridge
endif

ifeq ($(sysname), Darwin)
  CXX = clang++
  CXXFLAGS = -Ofast -ffast-math --std=c++17
endif

all : playoff train

common_src = hits.cpp bgnet.cpp move.cpp ttydisp.cpp bearoff.cpp human.cpp

playoff_src = playoff.cpp $(common_src)

playoff : $(playoff_src) bdata.o
	$(CXX) $(CXXFLAGS) $(playoff_src) bdata.o -o playoff


train_src = train.cpp $(common_src)

train : $(train_src)
	$(CXX) $(CXXFLAGS) $(train_src) bdata.o -o train

bearoff/bearoff.dat:
	$(MAKE) -C bearoff bearoff.dat

bdata.o : bdata.cpp bearoff/bearoff.dat
	$(CXX) $(CXXFLAGS) -c bdata.cpp



.PHONY : clean distclean

clean :
	-rm playoff train *.o

distclean :
	$(MAKE) -C bearoff distclean
	$(MAKE) clean
