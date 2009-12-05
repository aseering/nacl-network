PROGS := BroadcastReceiver.out \
BroadcastSender.out \
MulticastReceiver.out \
MulticastSender.out \
SigAction.out \
TCPEchoClient.out \
TCPEchoServer.out \
TCPEchoServer-Fork.out \
TCPEchoServer-ForkN.out \
TCPEchoServer-Fork-SIGCHLD.out \
TCPEchoServer-Select.out \
TCPEchoServer-Thread.out \
UDPEchoClient.out \
UDPEchoClient-Timeout.out \
UDPEchoServer.out \
UDPEchoServer-SIGIO.out
ECHO_SERVER_SRC := HandleTCPClient.c DieWithError.c CreateTCPServerSocket.c AcceptTCPConnection.c

all: $(PROGS)

#TCPEchoServer-Fork.out: TCPEchoServer-Fork.c 
#		gcc -o $@ $< $(ECHO_SERVER_SRC) 

%-Thread.out: %-Thread.c
		gcc -o $@ $< $(ECHO_SERVER_SRC) -lpthread

TCPEchoServe%.out: TCPEchoServe%.c 
	  gcc -o $@ $< $(ECHO_SERVER_SRC)

%.out: %.c 
		gcc -o $@ $< DieWithError.c



clean:
		 rm -f $(PROGS)

.PHONY: clean all