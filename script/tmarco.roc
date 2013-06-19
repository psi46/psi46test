vd 2500
pon
chipid 0
select 0
mdelay 500
clock40
mask

dac   1   4  Vdig        VD = 2.5V
dac   2  70  Vana
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



vthr  60
vcal  60

dac  20 80  VIref_ADC 160
dac  15 40  VOffsetOp
dac  17 120 VOffsetRO 

trc 20
tct 25   24@40MHz, 21@20MHz, 20@10MHz
ttk 16
wbc 21
cc   1
trep 40

set 26 0
set 43 2
set 41 b0100001  select ADC2 = SDATA

droc
seq b1000
run
udelay 100
dclear

mdelay 100

; -------------------------------------------------

seq b1000
run
udelay 100

seq b0111

cole :
pixe 0 : 0

cal 0 0
single
cald

seq b0111

cal 0 1
single
cald

cal 0 2
single
cald

cal 0 3
single
cald

cal 0 4
single
cald


flush
