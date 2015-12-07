OBJS = train.o hits.o net.o move.o ttydisp.o random.o bdata.o beardll.o playernet.o human.o game.o console.o
BEARGEN_OBJS = beargen.o random.o move.o console.o

#CPPFLAGS = -O4 -ffast-math -ffp-contract=fast -fvectorize
#CXX = clang++
CPPFLAGS = -Ofast -ffast-math -fassociative-math -ffp-contract=fast -Wdouble-promotion
CXX = g++

train : $(OBJS)
	$(CXX) -O3  -o train $(OBJS)

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
	-rm -f beargen train bearoff.dat *.o

.PHONY : clean all
