.PHONY: all clean distclean

UNAME := $(shell uname)

OBJS = cmd.o command.o pixel_dtb.o protocol.o psi46test.o rpc.o rpc_calls.o settings.o usb.o plot.o datastream.o chipdatabase.o defectlist.o pixelmap.o prober.o ps.o linux/rs232.o color.o error.o histo.o profiler.o scanner.o test.o test_dig.o rpc_error.o test_ana.o test_PROC600.o file.o cmd_dtb.o cmd_wafertest.o cmd_analyzer.o waferlist.o

ifeq ($(UNAME), Darwin)
CXXFLAGS = -g -Os -Wall -I/usr/local/include -Wno-logical-op-parentheses -I/usr/X11/include
LDFLAGS = -lftd2xx -lreadline -L/usr/local/lib -L/usr/X11/lib -lX11
endif

ifeq ($(UNAME), Linux)
CXXFLAGS = -g -Os -Wall -I/usr/local/include -I/usr/X11/include -pthread
# CXXFLAGS = -g -Os -Wall -Werror -I/usr/local/include -Wno-logical-op-parentheses -I/usr/X11/include -pthread
LDFLAGS = -lftd2xx -lreadline -L/usr/local/lib -L/usr/X11/lib -lX11 -pthread -lrt
endif

RPCGEN = ./rpcgen/rpcgen

#################
# PATTERN RULES #
#################
obj/%.o : %.cpp
	@mkdir -p obj/linux
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/%.d : %.cpp obj
	@mkdir -p obj/linux
	$(shell $(CXX) -MM $(CXXFLAGS) $< | awk -F: '{if (NF > 1) print "obj/"$$0; else print $0}' > $@)


###########
# TARGETS #
###########
all: bin/psi46test
	@true

obj:
	@mkdir -p obj/linux

bin:
	@mkdir -p bin

rpc_calls.cpp:
	make -C rpcgen
	$(RPCGEN) pixel_dtb.h -hrpc_calls.cpp > rpcgen.log

bin/psi46test: $(addprefix obj/,$(OBJS)) bin rpc_calls.cpp
	$(CXX) -o $@ $(addprefix obj/,$(OBJS)) $(LDFLAGS)

clean:
	rm -rf obj
	rm -rf rpc_calls.cpp

distclean: clean
	rm -rf bin

################
# DEPENDENCIES #
################
-include $(addprefix obj/,$(OBJS:.o=.d))
