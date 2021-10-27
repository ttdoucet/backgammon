sysname = $(shell uname -s)
archname = $(shell uname -m)

export CXXFLAGS = -Ofast -ffast-math --std=c++17 -iquote cmdline -iquote matrix -iquote stopwatch -MMD
export LDFLAGS = -L bearoff -lbearoff

ifeq ($(sysname), Linux)
  ifeq ($(archname), x86_64)
   export CXX = g++-10  -march=sandybridge
   # export CXX = clang++-10
  endif
endif

ifeq ($(sysname), Darwin)
  export CXX = /opt/homebrew/opt/llvm/bin/clang++
  # export CXX = clang++
  # export CXX = g++-11
endif

all : bearoff/libbearoff.a  playoff train

common_src = hits.cpp bgnet.cpp move.cpp ttydisp.cpp human.cpp
common_obj = $(common_src:.cpp=.o)


playoff : playoff.o $(common_obj)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

train : train.o $(common_obj)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

bearoff/libbearoff.a :
	$(MAKE) -C bearoff libbearoff.a

all_obj = $(common_obj) train.o playoff.o
dep = $(all_obj:.o=.d)
-include $(dep)

.PHONY : clean distclean

clean :
	-@rm playoff train *.o *.d 2>/dev/null || true

distclean :
	@$(MAKE) -C bearoff distclean
	@$(MAKE) clean
