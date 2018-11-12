TARGET_NAME = QueueBenchmark
OBJECS_NAME = orgQueue.o QueueBenchmark.o

LIBS = -lpmemobj -lpthread
CXXFLAGS=-ggdb -std=c++11

all: $(TARGET_NAME)	

$(TARGET_NAME) : $(OBJECS_NAME)
	g++ -o $(TARGET_NAME) $(OBJECS_NAME) -L/usr/local/lib/pmdk_debug $(LIBS)
	
%.o: %.cpp 
	g++ -o $@ -c $(CXXFLAGS) $<

clean:
	rm -rf *.o $(TARGET_NAME)
