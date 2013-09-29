OBJS = train.o hits.o net.o move.o ttydisp.o random.o bdata.o beardll.o playernet.o human.o game.o console.o

#CPPFLAGS = -O4 -fomit-frame-pointer -ffast-math -fstrict-aliasing -mtune=barcelona
#CPPFLAGS = -O3 -march=native -mfpmath=sse -Ofast -flto -march=native -funroll-loops -ffast-math -fomit-frame-pointer

#CPPFLAGS = -O3 -march=native -funroll-loops -ffast-math
CPPFLAGS = -O3 -march=native -funroll-loops -Wno-unused-result

#CXX = g++
#CXX = clang++

BEARGEN_OBJS = beargen.o random.o move.o console.o

train : $(OBJS)
	$(CXX) -O3 -o train $(OBJS)

beargen : $(BEARGEN_OBJS)
	$(CXX) -o beargen $(BEARGEN_OBJS)

all : beargen train

clean : 
	-rm -f train beargen *.o $(all)

.PHONY : clean all
