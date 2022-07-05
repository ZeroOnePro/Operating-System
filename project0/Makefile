TARGET	= pa0

all: pa0

CC = gcc

OBJECTS = pa0

C = pa0.c

CFLAGS = -W -Wall

pa0: pa0.c
	gcc -g -o $@ $^

.PHONY: clean
clean:
	rm -f *.o pa0

test:
	 $(CC) -c -o $(OBJECTS) $(C)       
	 $(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lm 
	 ./$(TARGET)
