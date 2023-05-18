CXX=g++
CXXFLAGS=
LFLAGS=


backtrace.o: backtrace.cpp
	$(CXX) $(CXXFLAGS) ./backtrace.cpp -o ./backtrace.o -ltcmalloc 
clean:
	-rm -rf *.o
	-rm -rf *.tmp
	-rm -rf *.heap
.PHONY: clean