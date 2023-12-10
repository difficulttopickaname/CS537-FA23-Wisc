CC = $(TOOLPREFIX)gcc
CFLAGS = -Wall -Werror -pedantic -std=gnu18
LOGIN = minhan
SUBMITPATH = ~cs537-1/handin/minhan/P3

.PHONY: all

all: wsh

wsh: wsh.c wsh.h
	$(CC) $(CFLAGS) $^ -o $@

run: wsh
	./wsh

pack: wsh.h wsh.c Makefile README.md
	tar -czvf $(LOGIN).tar.gz $^

pack-slip: wsh.h wsh.c Makefile README.md slipdays.txt
	tar -czvf $(LOGIN).tar.gz $^

submit: pack
	scp $(LOGIN).tar.gz $(SUBMITPATH)
	
