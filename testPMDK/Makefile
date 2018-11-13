TARGET_NAME1 = QueueBenchmark
OBJECS_NAME1 = orgQueue.o QueueBenchmark.o

TARGET_NAME2 = CreateQueueFile
OBJECS_NAME2 = CreateQueueFile.o orgQueue.o

LIBS = -lpmemobj -lpthread
CXXFLAGS=-ggdb -std=c++11

all: $(TARGET_NAME1)	$(TARGET_NAME2)

$(TARGET_NAME1) : $(OBJECS_NAME1)
	g++ -o $(TARGET_NAME1) $(OBJECS_NAME1) -L/usr/local/lib/pmdk_debug $(LIBS)
	
$(TARGET_NAME2) : $(OBJECS_NAME2)
	g++ -o $(TARGET_NAME2) $(OBJECS_NAME2) -L/usr/local/lib/pmdk_debug $(LIBS)
	
%.o: %.cpp 
	g++ -o $@ -c $(CXXFLAGS) $<

clean:
	rm -rf *.o $(TARGET_NAME1) $(TARGET_NAME2)
