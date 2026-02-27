# Makefle for UDP socket examples
#
# Saleem Bhatti <https://saleem.host.cs.st-andrews.ac.uk/>
#
# Jan 2024, Jan 2022, Jan 2021
# Checked February 2025, 2026 (sjm55)

CC		=clang
#CC	=gcc
CC-flags		=-Wall -g #-DDEBUG

PROGRAMS	= udp_sender

.SUFFIXES:	.c .o

.c.o:;	$(CC) $(CC-flags) -c $<

C-files		= UdpSocket.c \
	udp_sender.c


all:	$(PROGRAMS)

UdpSocket.c:	UdpSocket.h

udp_sender:	udp_sender.o UdpSocket.o
	$(CC) -o $@ $+

.PHONY:	clean

clean:;	rm -rf *.o $(PROGRAMS) *~
