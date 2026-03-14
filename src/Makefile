CC      = gcc
PROF    = -DLINUX
NOCRYPT =
C_FLAGS = -Wall -ggdb -O3 $(PROF) $(NOCRYPT)
L_FLAGS = -O2 $(PROF)

O_FILES = act_basic.o \
          ban.o \
          bit.o \
          comm.o \
          const.o \
          db.o \
          handler.o \
          interp.o \
          mem.o \
          random.o \
          recycle.o \
          save.o \
          ssm.o \
          string.o \
          update.o

all: ember

ember: $(O_FILES)
	rm -f ember
	$(CC) $(L_FLAGS) -o ember $(O_FILES) -lm -lws2_32

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	-rm -f *.o *~ Ember.patch *.orig *.rej \#*

distclean:
	-rm -f ../area/*~ ../area/*.txt
	-rm -f ../src/*~ ../*~
	-rm -f ../gods/* ../log/* ../player/*

src:
	-rm -f src.tar.gz
	-tar -czvf src.tar.gz Makefile *.c *.h
