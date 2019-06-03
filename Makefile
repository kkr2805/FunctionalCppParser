CC = g++
CPPFLAGS = -g --std=c++11 -I./src
LIBFLAGS = 
SRCS = ./src/main.cpp
OBJS = $(SRCS:./src/%.cpp=./build/%.o)

all: $(OBJS)
	$(CC) $(CPPFLAGS) -o main $^ $(LIBFLAGS)

./build/%.o: ./src/%.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $^ $(LIBFLAGS)

clean:
	rm -f ./build/* ./main


