CC = g++
CFLAGS = -pthread -o

all: B2 C1 C2 A1

B2: B2.cpp
	$(CC) $(CFLAGS) B2 B2.cpp

C1: C1.cpp
	$(CC) $(CFLAGS) C1 C1.cpp

C2: C2.cpp
	$(CC) $(CFLAGS) C2 C2.cpp

A1: A1.cpp
	$(CC) $(CFLAGS) A1 A1.cpp

run: $(FILE)
	./$(FILE)

clean:
	rm -f B2 C1 C2 A1
