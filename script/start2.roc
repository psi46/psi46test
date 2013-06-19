pon
chipid 0
select 0
mdelay 500
clock40
rocdefault
vthr  60
vcal  60

trc 20
tct 24   24@40MHz, 21@20MHz, 20@10MHz
ttk 10
wbc 21
cc   1
trep 40

set 26 0
set 43 2
set 41 b10100000
droc
seq b1111
loop
mdelay 100
cole 0
cole 1
pixe 0 0 0
cal 0 0
flush
