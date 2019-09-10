OBJS = train.o hits.o net.o move.o ttydisp.o random.o bdata.o bearoff.o human.o
BEARGEN_OBJS = beargen.o bearoff.o move.o

#CPPFLAGS = -O3 -ffast-math --std=c++17 -save-temps -g
CPPFLAGS = -O3 -ffast-math -fvectorize --std=c++17
CXX = clang++-5.0
#CXX = clang++-8

train : $(OBJS)
	$(CXX) -O3 -o train $(OBJS)

netcp.o : stopwatch.h

NETCP_OBJS = netcp.o net.o
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

distclean : clean
	-rm -f beargen train netcp bearoff.dat *.o

.PHONY : clean all
