fsel   0      40 MHz (T = 25ns)
clk    0 ns
sda   22 ns   0.75*T
ctr    0 ns   0.00*T
tin   13 ns   0.40*T
flush

seq b1111
trc 20
tct 37
ttk 10
wbc 44
cc   1
trep 40

pon
mdelay 500

probe 0 8
flush

set 41 b0100000
dfix 50

dtrig
run
mdelay 100
dread

mdelay 100
loop
flush
