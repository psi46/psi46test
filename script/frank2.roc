
pon
clk 10
sda 25
ctr 10
tin 15
pgset 0 b101000 10
pgset 1 b000100 40
pgset 2 b000010 16
pgset 3 b000001 0
rocaddr b1011
select b1011
d1 9
a1 0
a2 1
pgloop 1000
dac   1   4  Vdig        VD = 2.5V
dac   2 140  Vana
dac   3  40  Vsf
dac   4  12  Vcomp       VD = 2.5V

dac   7  60  VwllPr
dac   9  60  VwllPr
dac  10 117  VhldDel
dac  11  29  Vtrim
dac  12  60  VthrComp

dac  13  30  VIBias_Bus
dac  14   6  Vbias_sf
dac  22  99  VIColOr

dac  15  40  VoffsetOp
dac  17 140  VoffsetRO
dac  18 115  VIon

dac  19 100  Vcomp_ADC 100
dac  20 160  VIref_ADC 160

dac  25   2  Vcal
dac  26  30  CalDel

dac  $fe 40  WBC
dac  $fd  4  CtrlReg
vcal 60
vthr 60
wbc 40
cole :
pixe 10 10 0
cal 10 10
wbc 39
wbc 38
wbc 37
wbc 36
wbc 35
wbc 34

