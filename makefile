PORT=8080
all: testserver
	./testserver $(PORT)

testserver: server.c
	gcc -o testserver server.c