cpp = g++
cpp_flags = -Ofast -ffast-math --std=c++17

all : playoff train

common_src = hits.cpp bgnet.cpp move.cpp ttydisp.cpp bearoff.cpp human.cpp

playoff_src = playoff.cpp $(common_src)

playoff : $(playoff_src) bdata.o
	$(cpp) $(cpp_flags) $(playoff_src) bdata.o -o playoff


train_src = train.cpp $(common_src)

train : $(train_src)
	$(cpp) $(cpp_flags) $(train_src) bdata.o -o train

bearoff.dat : bearoff/beargen
	bearoff/beargen > bearoff.dat

bdata.o : bdata.cpp bearoff.dat
	$(cpp) $(cpp_flags) -c bdata.cpp

clean :
	-rm playoff train
