.PHONY: all clean install

CLIBS = -L./linux/lib -lftd2xx -lreadline -ltermcap
CFLAGS = -DWAFERTESTER

all: psi46test

clean:
	rm -f $(shell ls *.o) psi46test

install: all
	install -m 0755 psi46test /usr/local/bin
	install -m 0755 lib/linux/libftd2xx.so.0.4.16 /usr/local/lib
	install -m 0755 lib/linux/libftd2xx.so.0 /usr/local/lib
	install -m 0755 lib/linux/libftd2xx.so /usr/local/lib
	@echo
	@echo '****************************************************************************'
	@echo '* Make sure that /usr/local/bin is in your PATH environment variable and   *'
	@echo '* that /usr/local/lib is in your LD_LIBRARY_PATH.                          *'
	@echo '*                                                                          *'
	@echo '* If that is not the case you can add the following lines to your          *'
	@echo '* ~/bashrc file (if you use bash, otherwise consult the manual of your     *'
	@echo '* shell):                                                                  *'
	@echo '*                                                                          *'
	@echo '*     export PATH=$$PATH:/usr/local/bin                                     *'
	@echo '*     export LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:/usr/local/lib               *'
	@echo '*                                                                          *'
	@echo '* To make the USB interface accessible for all users, modify your udev     *'
	@echo '* rules (in /etc/udev/rules.d) and set the permission mode for usb_device  *'
	@echo '* to 0666.                                                                 *'
	@echo '*                                                                          *'
	@echo '* To get access to the serial interface the permissions for the device     *'
	@echo '* /dev/ttyS0 must be set to 0666 as well.                                  *'
	@echo '****************************************************************************'
	

psi46test: psi46test.o psi46_tb.o protocol.o settings.o prober.o cmd.o command.o test.o analog.o pixelmap.o histo.o chipdatabase.o scanner.o ps.o error.o color.o mmatest.o decoder.o sectortable.o usb.o rs232.o
	g++ -o $@ $^ $(CLIBS)

psi46test.o: psi46test.cpp psi46test.h psi46_tb.h linux/usb.h linux/ftd2xx.h linux/WinTypes.h settings.h prober.h protocol.h pixelmap.h scanner.h test.h mmatest.h sectortable.h chipdatabase.h error.h decoder.h
	g++ -c $<
psi46_tb.o: psi46_tb.cpp psi46_tb.h linux/usb.h linux/ftd2xx.h linux/WinTypes.h remotecalls.inc remotecalls_xraytest.inc remotecalls_chiptest.inc remotecalls_modultest.inc
	g++ -c $<
protocol.o: protocol.cpp protocol.h psi46test.h psi46_tb.h linux/usb.h linux/ftd2xx.h linux/WinTypes.h settings.h prober.h pixelmap.h scanner.h test.h mmatest.h sectortable.h chipdatabase.h error.h decoder.h
	g++ -c $<
settings.o: settings.cpp settings.h
	g++ -c $<
prober.o: prober.cpp rs232.h prober.h
	g++ -c $<
cmd.o: cmd.cpp psi46test.h psi46_tb.h linux/usb.h linux/ftd2xx.h linux/WinTypes.h settings.h prober.h protocol.h pixelmap.h scanner.h test.h mmatest.h sectortable.h chipdatabase.h error.h decoder.h command.h defectlist.h
	g++ -c $<
command.o: command.cpp command.h
	g++ -c $<
ChipData.o: ChipData.cpp ChipData.h
	g++ -c $<
test.o: test.cpp psi46test.h psi46_tb.h linux/usb.h linux/ftd2xx.h linux/WinTypes.h settings.h prober.h protocol.h pixelmap.h scanner.h test.h mmatest.h sectortable.h chipdatabase.h error.h decoder.h analog.h
	g++ -c -DWAFERTESTER $<
analog.o: analog.cpp psi46test.h psi46_tb.h linux/usb.h linux/ftd2xx.h linux/WinTypes.h settings.h prober.h protocol.h pixelmap.h scanner.h test.h mmatest.h sectortable.h chipdatabase.h error.h decoder.h analog.h histo.h
	g++ -c $<
pixelmap.o: pixelmap.cpp pixelmap.h scanner.h chipdatabase.h error.h
	g++ -c -DWAFERTESTER $<
histo.o: histo.cpp histo.h protocol.h
	g++ -c $<
chipdatabase.o: chipdatabase.cpp ps.h color.h chipdatabase.h error.h pixelmap.h scanner.h
	g++ -c $<
scanner.o: scanner.cpp scanner.h
	g++ -c $<
ps.o: ps.cpp ps.h
	g++ -c $<
error.o: error.cpp error.h
	g++ -c $<
color.o: color.cpp color.h
	g++ -c $<
mmatest.o: mmatest.cpp psi46test.h psi46_tb.h linux/usb.h linux/ftd2xx.h linux/WinTypes.h settings.h prober.h protocol.h pixelmap.h scanner.h test.h mmatest.h sectortable.h chipdatabase.h error.h decoder.h
	g++ -c $<
decoder.o: decoder.cpp decoder.h psi46test.h psi46_tb.h linux/usb.h linux/ftd2xx.h linux/WinTypes.h settings.h prober.h protocol.h pixelmap.h scanner.h test.h mmatest.h sectortable.h chipdatabase.h error.h
	g++ -c $<
sectortable.o: sectortable.cpp sectortable.h
	g++ -c $<
usb.o: linux/usb.cpp linux/usb.h linux/ftd2xx.h linux/WinTypes.h
	g++ -c $<
rs232.o: linux/rs232.cpp
	g++ -c $<
