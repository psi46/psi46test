modtiming40
modpon
tbmsingle

select :
rocdefault

dlevel -700
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
vthr 100
ctl  b0100 high range, full speed readout

cal 0 0
cole 0
cole 1
pixe 0 0 0
loop

probe 0 8
probe 1 6
dmod2
mmaaout 23
flush

dtrig
mdelay 200
dread
dmodcnt
mdelay 200
stop
mdelay 200
dacdac 0
