CC = gcc
CFLAGS = -W -Wall
C = system.c
OBJECTS = system.o
TARGET = system

clean : 
	rm *.o $(TARGET)

test :
	$(CC) -c -o $(OBJECTS) $(C)       
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lm 
	./$(TARGET)

