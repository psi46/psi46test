vd 2500
pon
chipid 0
select 0
mdelay 500
clock40
rocdefault
vthr  60
vcal  60

dac  20 80  VIref_ADC 160
dac  15 40  VOffsetOp
dac  17 120 VOffsetRO 

pgset 0 b1000 22
pgset 1 b0100 26
pgset 2 b0010 16
pgset 3 b0001  0

wbc 21

set 26 0
set 43 2
set 41 b0100001  select ADC2 = SDATA
droc
pgloop 10000

probe 1 6
mdelay 100
cole :
pixe 24 6 0
pixe 41 30 0
cal 24 6
cal 41 30
flush
