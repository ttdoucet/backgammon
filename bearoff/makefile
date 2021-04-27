CXX = clang++
CXXFLAGS = -Ofast -ffast-math --std=c++17 -iquote .. -iquote ../cmdline

beargen : 
	$(CXX) $(CXXFLAGS) -iquote .. beargen.cpp bearoff.cpp ../move.cpp -o beargen

bearoff.dat : beargen
	./beargen > bearoff.dat

libbearoff.a : bearoff.dat bdata.cpp bearoff.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c bearoff.cpp bdata.cpp
	$(AR) rcs libbearoff.a bdata.o bearoff.o

.PHONY : clean

clean:
	@rm beargen bearoff.dat *.o *.a 2>/dev/null || true

distclean :
	@make clean

