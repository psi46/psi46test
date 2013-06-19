init
;-- power on
vd 2500
pon
chipid 0
select 0
mdelay 500
clock40

;--- set ROC dacs ---------------------
mask

dac   1   8  ;Vdig
vana     80  ;Vana
dac   3  20  ;Vsf
dac   4  12  ;Vcomp

dac   7  60  ;VwllPr
dac   9  60  ;VwllPr
dac  10 255  ;VhldDel
dac  11 100  ;Vtrim
vthr     60  ;VthrComp

dac  13  30  ;VIBias_Bus
dac  14   6  ;Vbias_sf
dac  22  99  ;VIColOr

dac  15  80  ;VoffsetOp
dac  17 140  ;VoffsetRO
dac  18 115  ;VIon

dac  19 100  ;Vcomp_ADC 100
dac  20 150  ;VIref_ADC

vcal    200  ;Vcal
dac  26  39  ;CalDel

wbc      20  ;WBC

dac  $fd  4  ;CtrlReg  CAL high range
udelay 500

;--- setup testboard DAQ --------------
set 26 0
set 43 2
set 41 b0100001  ;select ADC2 = SDATA
droc

;--- enable pixel and calibrate -------
cole :
pixe : :  0
cal  10:11 0:4

;--- setup pattern --------------------
pgset 0 b1000 22  ;RES
pgset 1 b0100 26  ;CAL
pgset 2 b0010 16  ;TRG
pgset 3 b0001  0  ;TOK
pgsingle

udelay 1000

pgset 0 b1000 22  ;RES
pgset 1 b0100 26  ;CAL
pgset 2 b0010 16  ;TRG
pgset 3 b0001  0  ;TOK
pgloop 10000

cald
vthr 50

cal 10 0:5
mdelay 10
d
cal 10 0:8
mdelay 10
d
cal 10 0:9
mdelay 10
d
cal 10 0:10
mdelay 10
d
cal 10 0:11
mdelay 10
d
cal 10 0:12
mdelay 10
d
cal 10 0:13
mdelay 10
d

flush
