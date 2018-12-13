all:
	gcc -pthread client.c -o client
	gcc -pthread server.c -o server
clean: rm -f client server
