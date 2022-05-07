SRC = ${CURDIR}/src
CC = g++
CFLAGS = -Ofast -fopenmp

all:  read.o write.o dfs_search.o
	$(CC) $(CFLAGS) -o dfs_search dfs_search.o read.o write.o
	
read.o: $(SRC)/read.cpp $(SRC)/read.h
	$(CC) -c $(SRC)/read.cpp

write.o: $(SRC)/write.cpp $(SRC)/write.h
	$(CC) -c $(SRC)/write.cpp
	
dfs_search.o: $(SRC)/dfs_search.cpp
	$(CC) $(CFLAGS) -c $(SRC)/dfs_search.cpp -o dfs_search.o
	
clean:
	rm -f *.o
