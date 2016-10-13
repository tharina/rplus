LLVM_CONFIG ?= llvm-config

CXXFLAGS ?= -O3 -DNDEBUG -g -march=native
#override CXXFLAGS += -std=c++1y -Wall -Wextra -Werror -pedantic
override CXXFLAGS += -std=c++1y -Wall -Wextra -pedantic
override LDFLAGS  += -lpapi

SRCS := $(filter-out %_malloc.cpp,$(wildcard framework/*.cpp))
OBJS := $(patsubst %.cpp,%.o,$(SRCS))
HDRS := $(wildcard framework/*.h) range_search.h
IMPL := $(wildcard *.h)
OBJS_MALLOC := $(subst bench.o,bench_malloc.o,$(OBJS)) \
	$(patsubst %.cpp,%.o,$(wildcard framework/*_malloc.cpp)) \
	malloc_count/malloc_count.o

.PHONY: all clean

all: bench bench_malloc

clean:
	rm -f framework/*.o malloc_count/malloc_count.o bench bench_malloc debug sanitize

malloc_count/malloc_count.o: malloc_count/malloc_count.c  malloc_count/malloc_count.h
	$(CC) -O2 -Wall -Werror -g -c -o $@ $<

framework/bench.o: $(IMPL)

framework/%.o: framework/%.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

framework/bench_malloc.o: framework/bench.cpp $(HDRS) $(IMPL)
	$(CXX) $(CXXFLAGS) -DMALLOC_INSTR -o $@ -c $<

bench: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

bench_malloc: $(OBJS_MALLOC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -ldl

test: $(IMPL) test.cpp
	$(CXX) $(CXXFLAGS) -o test test.cpp

sanitize: $(SRCS) $(HDRS) $(IMPL)
	$(shell $(LLVM_CONFIG) --bindir)/clang++ $(CXXFLAGS) -stdlib=libc++ \
		-Wl,-rpath=$(shell $(LLVM_CONFIG) --libdir) -fsanitize=address \
		-o $@ $(SRCS) $(LDFLAGS) -lc++abi
	ASAN_SYMBOLIZER_PATH=$(shell $(LLVM_CONFIG) --bindir)/llvm-symbolizer ./$@ -e 16 -m time

