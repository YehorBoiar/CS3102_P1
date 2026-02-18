# Makefle for UDP socket examples
#
# Saleem Bhatti <https://saleem.host.cs.st-andrews.ac.uk/>
#
# Jan 2024, Jan 2022, Jan 2021
# Checked February 2025, 2026 (sjm55)

CC		=clang
#CC	=gcc
CC-flags		=-Wall -g #-DDEBUG

PROGRAMS	=udp-server-1 udp-client-1 \
	udp-server-2 udp-client-2 \
	udp-chat-1 \
	udp-chat-2 \
	udp-server-async \
	timer \
	udp-chat-3 \
	udp_sender

.SUFFIXES:	.c .o

.c.o:;	$(CC) $(CC-flags) -c $<

C-files		=udp-server-1.c udp-client-1.c \
	UdpSocket.c \
	udp-server-2.c udp-client-2.c \
	udp-chat-1.c \
	udp-chat-2.c \
	udp-server-async.c \
	timer.c \
	udp-chat-3.c \
	udp_sender.c


all:	$(PROGRAMS)

UdpSocket.c:	UdpSocket.h

udp-client-2.c:	UdpSocket.h

udp-server-2.c:	UdpSocket.h

udp-chat-1.c:	UdpSocket.h

udp-chat-2.c:	UdpSocket.h

udp-server-async.c:	UdpSocket.h

udp-chat-3.c:	UdpSocket.h

udp-server-1:	udp-server-1.o UdpSocket.o
	$(CC) -o $@ $+

udp-client-1:	udp-client-1.o UdpSocket.o
	$(CC) -o $@ $+

udp-server-2:	udp-server-2.o UdpSocket.o
	$(CC) -o $@ $+

udp-client-2:	udp-client-2.o UdpSocket.o
	$(CC) -o $@ $+

udp-server-async:	udp-server-async.o UdpSocket.o
	$(CC) -o $@ $+

timer:	timer.o
	$(CC) -o $@ $+

udp-chat-1:	udp-chat-1.o UdpSocket.o
	$(CC) -o $@ $+

udp-chat-2:	udp-chat-2.o UdpSocket.o
	$(CC) -o $@ $+

udp-chat-3:	udp-chat-3.o UdpSocket.o
	$(CC) -o $@ $+

udp_sender:	udp_sender.o UdpSocket.o
	$(CC) -o $@ $+

.PHONY:	clean

clean:;	rm -rf *.o $(PROGRAMS) *~
