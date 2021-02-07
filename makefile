sysname = $(shell uname -s)

CXXFLAGS = -Ofast -ffast-math --std=c++17 -L bearoff

ifeq ($(sysname), Linux)
  CXX = g++-10
  CXXFLAGS += -march=sandybridge -mtune=sandybridge
endif

ifeq ($(sysname), Darwin)
  CXX = clang++
endif

all : bearoff/libbearoff.a  playoff train

common_src = hits.cpp bgnet.cpp move.cpp ttydisp.cpp human.cpp

playoff : playoff.cpp $(common_src)
	$(CXX) $(CXXFLAGS) $^ -lbearoff -o playoff

train : train.cpp $(common_src)
	$(CXX) $(CXXFLAGS) $^ -l bearoff -o train

bearoff/libbearoff.a :
	$(MAKE) -C bearoff libbearoff.a


.PHONY : clean distclean

clean :
	-rm playoff train *.o

distclean :
	$(MAKE) -C bearoff distclean
	$(MAKE) clean
