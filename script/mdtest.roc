modtiming40
modpon
tbmdual
select :
rocdefault

vthr  60
vcal  60

mmaaout 22
probe 0 8
probe 1 11

trc 20
tct 24   24@40MHz, 21@20MHz, 20@10MHz
ttk 10
wbc 21
cc   1
trep 40

daqinit 10000
dlevel  -500
dmod2
dena
dclear
dstart
set 26 0    marlon trigger delay
set 41 b00100010
set 43 b11  clear event timer, aquire reset
udelay 10
daqena

set 41 b00101010
seq b1110
loop
mdelay 100
cole 0
cole 1
pixe 0 0 0
cal 0 0
flush


mdelay 500


stop
cdelay 100
set 41 b00100010
daqdis
daqgetptr
dclear
flush

daqsave aaa.txt
daqdone
flush
