.PHONY: all clean distclean

OBJS = cmd.o command.o pixel_dtb.o protocol.o psi46test.o rpc.o rpc_calls.o settings.o usb.o plot.o datastream.o analyzer.o

#CXXFLAGS = -g -Os -Wall -Werror -I/usr/local/include -Wno-logical-op-parentheses -I/usr/X11/include
# temporarily removed -Werror
CXXFLAGS = -g -Os -Wall -I/usr/local/include -Wno-logical-op-parentheses -I/usr/X11/include
#LINUX COMMAND:
#CXXFLAGS = -g -Os -Wall -Werror -I/usr/local/include -Wno-logical-op-parentheses -I/usr/X11/include -pthread
LDFLAGS = -lftd2xx -lreadline -L/usr/local/lib -L/usr/X11/lib -lX11
#LINUX COMMAND
#LDFLAGS = -lftd2xx -lreadline -L/usr/local/lib -L/usr/X11/lib -lX11 -pthread -lrt

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
	rpcgen pixel_dtb.h -hrpc_calls.cpp

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
