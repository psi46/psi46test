; clock stretch test PLL
;

init
pgdis

;-- start up --------------------------
fsel   0      40 MHz (T = 25ns)
clk   24 ns
sda   19 ns
ctr   11 ns
tin   11 ns
flush
udelay 10

vd 2500 mV
va 1500 mV
pon
chipid 0
select 0
mdelay 500

mask         ;mask all pixels and dcols
flush

;--- set ROC dacs ---------------------
dac   1   0  ;Vdig
vana     80  ;Vana
dac   3  20  ;Vsf
dac   4  12  ;Vcomp

dac   7  60  ;VwllPr
dac   9  60  ;VwllPr
dac  10 255  ;VhldDel
dac  11 100  ;Vtrim
vthr     50  ;VthrComp

dac  13  30  ;VIBias_Bus
dac  14   6  ;Vbias_sf
dac  22  99  ;VIColOr

dac  15  80  ;VoffsetOp
dac  17 140  ;VoffsetRO
dac  18 115  ;VIon

dac  19 100  ;Vcomp_ADC 100
dac  20 150  ;VIref_ADC

vcal    180  ;Vcal
dac  26  39  ;CalDel

dac  $fd  4  ;CtrlReg  CAL high range

wbc     240  ;WBC

pgset 0 b1000 0 ;update WBC
pgsingle
udelay 100

;--- setup testboard DAQ --------------
set 26 0
set 43 2
set 41 b0100001  ;select ADC2 = SDATA
droc

;--- enable pixel ---------------------
cole :
pixe : :  0

;--- reset ROC ------------------------
pgset 0 b1000  0
pgloop 10000
udelay 1000
pgdis
flush

;--- setup clock stretch --------------
stretch 2 200 100  ;adc error

;--- setup pattern --------------------

pgset 0 b1000  22  ;RES on
pgset 1 b0100 246  ;CAL (=WBC+6)
pgset 2 b0010 250  ;TRG
pgset 3 b0001   0  ;TOK

cald
cal 12 24
;dtrig
;pgsingle
;udelay 100
;dreadd 2
pgloop 10000

flush
