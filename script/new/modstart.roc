modtiming40
modpon
tbmsingle

select :
rocdefault

dlevel -500
set 41 b0100010

seq b1111
trc 20
tct 37   37@40MHz, 34@20MHz, 33@10MHz
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
