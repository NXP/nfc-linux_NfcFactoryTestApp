CC=gcc

CFILES=NfcFactoryTestApp.c

CFILES_PN5XX= tml_pn5xx.c
CFILES_ALT= tml_alt.c
CFILES_DUMMY= tml_dummy.c

INCLUDES= -I/usr/include/ -I./

LIBS=

CFLAGS=-Wall $(INCLUDES)

pn5xx:
	$(CC) -o NfcFactoryTestApp $(CFLAGS) $(CFILES) $(CFILES_PN5XX) $(LIBS)

alt:
	$(CC) -o NfcFactoryTestApp $(CFLAGS) $(CFILES) $(CFILES_ALT) $(LIBS)

dummy:
	$(CC) -o NfcFactoryTestApp $(CFLAGS) $(CFILES) $(CFILES_DUMMY) $(LIBS)

clean:
	rm NfcFactoryTestApp
