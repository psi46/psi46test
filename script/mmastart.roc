fsel   0      40 MHz (T = 25ns)
clk    5 ns
sda   27 ns   0.75*T
ctr    5 ns   0.00*T
tin   18 ns   0.40*T
rda    19     13...24
mdelay 100

mmaaout 22 23 22 23

modpon
modsel 28
tbmset $E4 $F0    Init TBM, Reset ROC
tbmset $F4 $F0
tbmset $E0 $01    ReadOutSpeed = full
tbmset $F0 $01
tbmset $E2 $C0    Mode = Pre-Calibration
tbmset $F2 $C0
tbmset $E8 $00    Enable TBM A
tbmset $F8 $00    Enable TBM B
tbmset $EA $c0    Analog Input Amplifier Bias
tbmset $EC $c0    Analog Output Driver Bias
tbmset $EE $c0    Analog Output DAC Gain
mdelay 100

select 0
rocdefault

dmod2
dlevel -600
set 41 b0100010

seq b1110
trc 20
tct 47   37@40MHz, 34@20MHz, 33@10MHz
ttk 10
wbc 44
cc   1
trep 40

select 0
vcal 120
ctl  b0100 high range, full speed readout

cal 0 0
cole 0
cole 1
pixe 0 0 0
loop

probe 0 8
flush
