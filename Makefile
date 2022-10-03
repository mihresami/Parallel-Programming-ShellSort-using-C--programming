CXX= g++
CXXFLAGS=-w
LDFLAGS = -lpthread

all: shellsort shellsort-parallel

shellsort: main.cc Sorters.hh ShellSorter.cc HRTimer.hh
	$(CXX) $(CXXFLAGS) -DSHELL main.cc ShellSorter.cc -o shellsort $(LDFLAGS)

shellsort-parallel: main.cc Sorters.hh ShellSorter.cc HRTimer.hh
	$(CXX) $(CXXFLAGS) -DSHELL -DPARALLEL main.cc ShellSorter.cc -o shellsort-parallel $(LDFLAGS)

clean:
	rm -f shellsort
	rm -f shellsort-parallel
	rm -f *~
                                                                                         
         
