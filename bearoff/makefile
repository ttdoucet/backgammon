CXX = clang++
CXXFLAGS = -Ofast -ffast-math --std=c++17

src = beargen.cpp bearoff.cpp move.cpp

#beargen : $(src)
#	$(CXX) $(CXXFLAGS) -I.. $(src) -o beargen

beargen : 
	$(CXX) $(CXXFLAGS) -I.. beargen.cpp bearoff.cpp ../move.cpp -o beargen

bearoff.dat : beargen
	./beargen > bearoff.dat

.PHONY : clean

clean:
	-rm beargen bearoff.dat *.o

distclean :
	make clean

