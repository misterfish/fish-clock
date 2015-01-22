FLAGS_C = -std=c99 -lm -lpthread
#FLAGS_C = -std=c99 -lm -lpthread -fPIC
FLAGS_SHARED = -fPIC -shared

all: fish-util.o libfish-util.so
	
fish-util.o: fish-util.c
	gcc -Wall $(FLAGS_C) -c fish-util.c -o fish-util.o

libfish-util.so: fish-util.c
	gcc $(FLAGS_SHARED) $(FLAGS_C) fish-util.c -o libfish-util.so

clean: 
	rm -f *.o
