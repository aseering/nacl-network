CC=gcc
CFLAGS=-lssl
RM=rm

.PHONY: all clean runserver

TCPResponder: TCPEchoValidateResponder.c HandleTCPClient-Validate.c nacl_socks_server.c DieWithError.c nacl_socks_server.h
	$(CC) -o TCPResponder -O3 -Wall TCPEchoValidateResponder.c HandleTCPClient-Validate.c nacl_socks_server.c DieWithError.c $(CFLAGS)

clean:
	$(RM) -f TCPResponder

runserver: all
	./TCPResponder 1123

