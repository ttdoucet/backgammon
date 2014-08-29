OBJS = train.o hits.o net.o move.o ttydisp.o random.o bdata.o beardll.o playernet.o human.o game.o console.o
BEARGEN_OBJS = beargen.o random.o move.o console.o

CPPFLAGS = -O3 -march=native -ffast-math -funroll-loops -Wno-unused-result
CXX = clang++

train : $(OBJS)
	$(CXX) -O3 -o train $(OBJS)

beargen : $(BEARGEN_OBJS)
	$(CXX) -o beargen $(BEARGEN_OBJS)

bdata.o : bdata.cpp bearoff.dat

bearoff.dat :
	make beargen
	./beargen > bearoff.dat

all : beargen train

clean : 
	-rm -f beargen train *.o

distclean : 
	-rm -f beargen train bearoff.dat *.o $(all)


.PHONY : clean all
