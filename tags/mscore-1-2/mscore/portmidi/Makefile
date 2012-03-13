# MAKEFILE FOR PORTMIDI AND PORTTIME


# For debugging, define PM_CHECK_ERRORS
# PMFLAGS = -DPM_CHECK_ERRORS -DUSE_DLL_FOR_CLEANUP
PMFLAGS = -DNEWBUFFER -DPM_CHECK_ERRORS
# Otherwise do not define PM_CHECK_ERRORS
# PMFLAGS =

versions = pm_win/pmwinmm.o
ALSALIB = -lwinmm

# Use this for linux alsa (0.9x) version
#versions = pm_linux/pmlinuxalsa.o
#ALSALIB = -lasound
#VFLAGS = -DPMALSA

# Use this for null (a dummy implementation for no Midi I/O:
# versions = pmlinuxnull.o
# ALSALIB =
# VFLAGS = -DPMNULL

pmdll = pm_win/pm_dll.dll
pmdlllib = pm_win/libpmdll.a
pmdllobj = pm_win/pmdll.o

pmlib = pm_win/libportmidi.a

ptlib = porttime/libporttime.a

CC = gcc
CFLAGS = $(VFLAGS) $(PMFLAGS) -g -Ipm_common -Iporttime

pmobjects = pm_common/pmutil.o $(versions) pm_win/pmwin.o  \
	pm_common/portmidi.o

ptobjects = porttime/porttime.o porttime/ptwinmm.o

current: all

all: $(pmlib) $(ptlib) pm_test/test pm_test/sysex pm_test/midithread \
#	pm_test/latency pm_test/midithru

$(pmdllobj): pm_win/pmdll.c
	$(CC) -DPM_LIBRARY -o $@ -c $^

$(pmdll) $(pmdlllib): $(pmdllobj)
	dlltool -D pm_dll.dll -e pm_win/pmdll_exports.o -l $(pmdlllib) $(pmdllobj)
	$(CC) -DPM_LIBRARY -mdll $(pmdllobj) pm_win/pmdll_exports.o -o $(pmdll)

$(pmlib): $(pmobjects)
	ar -cr $(pmlib) $(pmobjects)

$(ptlib): $(ptobjects)
	ar -cr $(ptlib) $(ptobjects)

pm_test/test: Makefile pm_test/test.o $(pmlib) $(ptlib) $(pmdlllib)
	$(CC) $(CFLAGS) pm_test/test.c -o pm_test/test $(pmlib) $(ptlib) $(pmdlllib) $(ALSALIB)

pm_test/sysex: Makefile pm_test/sysex.o $(pmlib) $(ptlib) $(pmdlllib)
	$(CC) $(CFLAGS) pm_test/sysex.c -o pm_test/sysex $(pmlib) $(ptlib) $(pmdlllib) $(ALSALIB)

pm_test/midithread: Makefile pm_test/midithread.o $(pmlib) $(ptlib) $(pmdlllib)
	$(CC) $(CFLAGS) pm_test/midithread.c -o pm_test/midithread \
        $(pmlib) $(ptlib) $(ALSALIB) $(pmdlllib)

pm_test/latency: Makefile $(ptlib) pm_test/latency.o
	$(CC) $(CFLAGS) pm_test/latency.c -o pm_test/latency $(pmlib) $(ptlib) \
        $(ALSALIB) -lpthread -lm

pm_test/midithru: Makefile $(ptlib) pm_test/midithru.o
	$(CC) $(CFLAGS) pm_test/midithru.c -o pm_test/midithru $(pmlib) $(ptlib) \
        $(ALSALIB) -lpthread -lm

clean:
	rm -f *.o *~ core* */*.o */*~ */core* pm_test/*/pm_dll.dll $(pmdll)
	rm -f *.opt *.ncb *.plg pm_win/Debug/pm_dll.lib pm_win/Release/pm_dll.lib
	rm -f pm_test/*.opt pm_test/*.ncb

cleaner: clean

cleanest: cleaner
	rm -f $(pmlib) $(ptlib) pm_test/test pm_test/sysex pm_test/midithread
	rm -f pm_test/latency pm_test/midithru

backup: cleanest
	cd ..; zip -r portmidi.zip portmidi
