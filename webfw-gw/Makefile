BS=$(shell pkg-config --cflags --libs glib-2.0 libcurl)
CFLAGS=-Wall -g -export-dynamic -lnetfilter_queue
SRC=main.c netfilter.c listman.c checkweb.c
webfw: netfilter.c main.c listman.c checkweb.c
	gcc -o webfw $(SRC) $(LIBS) $(CFLAGS) $(BS)
clean:
	(rm -f webfw listman *~)

