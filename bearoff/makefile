CXX = clang++
CXXFLAGS = -Ofast -ffast-math --std=c++17

beargen : 
	$(CXX) $(CXXFLAGS) -iquote .. beargen.cpp bearoff.cpp ../move.cpp -o beargen

bearoff.dat : beargen
	./beargen > bearoff.dat

.PHONY : clean

clean:
	-rm beargen bearoff.dat *.o

distclean :
	make clean

