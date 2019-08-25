OBJS = train.o hits.o net.o move.o ttydisp.o random.o bdata.o beardll.o playernet.o human.o game.o mathfuncs.o console.o
BEARGEN_OBJS = beargen.o random.o move.o console.o

#CPPFLAGS = -O3 -ffast-math -fvectorize --std=c++17 -save-temps -g
CPPFLAGS = -O3 -ffast-math -fvectorize --std=c++17
CXX = clang++-5.0
#CXX = clang++-8

train : $(OBJS)
	$(CXX) -O3 -o train $(OBJS)

netcp.o : stopwatch.h

NETCP_OBJS = netcp.o net.o mathfuncs.o console.o
netcp : $(NETCP_OBJS)
	$(CXX) -O3  -o netcp $(NETCP_OBJS)

beargen : $(BEARGEN_OBJS)
	$(CXX) -o beargen $(BEARGEN_OBJS)

bdata.o : bdata.cpp bearoff.dat

bearoff.dat :
	make beargen
	./beargen > bearoff.dat

all : beargen train

clean : 
	-rm -f beargen train *.o *.bc *.s *.ii

distclean : 
	-rm -f beargen train bearoff.dat *.o

.PHONY : clean all
