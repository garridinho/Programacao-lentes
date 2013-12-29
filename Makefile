DB_FLAGS	= -Wall -g -ggdb 
#DB_FLAGS	= -O3

CFLAGS_GTK	= `pkg-config --cflags glib-2.0` `pkg-config --cflags gtk+-2.0`

INC_FLAGS	= 
LINK_FLAGS	= 
LINK_GTK	= `pkg-config --libs glib-2.0` `pkg-config --libs gtk+-2.0`
LINK_LIBS	= $(LINK_GTK) -lm

LOPTS		=

CFLAGS		= $(DB_FLAGS) $(INC_FLAGS) $(CFLAGS_GTK)
LFLAGS		= $(DB_FLAGS) $(LINK_FLAGS) $(LINK_GTK)

CC		= gcc
CP		= cp
RM		= rm
LS		= ls

PROGS		= Menu tipo_de_janelas_a_escolher

all: comp link

link: 

comp: comp1

comp1: 
	for fname in $(PROGS); do $(CC) $(CFLAGS) -c $$fname.c ; done

link: link1

link1: 
	for fname in $(PROGS); do $(CC) $(LFLAGS) -o $$fname $$fname.o $(LINK_LIBS) ; done

clean:
	$(RM) -f *~ *.o $(PROGS)
