; maximum wave jump (pixel 0 to 159)
; @ vdig = 0
;
;--- 318 samples read -----------------
;Header:7F8 #pixel:52
;[ 0  0 141] 00.424  [ 1  0 141] 00.425
;[ 2  0 142] 01.424  [ 3  0 142] 01.425
;[ 4  0 142] 02.424  [ 5  0 142] 02.425
;[ 6  0 144] 03.424  [ 7  0 144] 03.425
;[ 8  0 144] 04.424  [ 9  0 144] 04.425
;[10  0 144] 05.424  [11  0 144] 05.425
;[12  0 144] 10.424  [13  0 144] 10.425
;[14  0 144] 11.424  [15  0 144] 11.425
;[16  0 146] 12.424  [17  0 146] 12.425
;[18  0 146] 13.424  [19  0 146] 13.425
;[20  0 146] 14.424  [21  0 147] 14.425
;[22  0 147] 15.424  [23  0 147] 15.425
;[24  0 148] 20.424  [25  0 149] 20.425
;[26  0 150] 21.424  [27  0 150] 21.425
;[28  0 150] 22.424  [29  0 150] 22.425
;[30  0 150] 23.424  [31  0 150] 23.425
;[32  0 151] 24.424  [33  0 152] 24.425
;[34  0 152] 25.424  [35  0 152] 25.425
;[36  0 152] 30.424  [37  0 152] 30.425
;[38  0 152] 31.424  [39  0 152] 31.425
;[40  0 152] 32.424  [41  0 152] 32.425
;[42  0 152] 33.424  [43  0 152] 33.425
;[44  0 152] 34.424  [45  0 152] 34.425
;[46  0 152] 35.424  [47  0 152] 35.425
;[48  0 152] 40.424  [49  0 152] 40.425
;[50  0 152] 41.424  [51  0 152] 41.425

init
pgdis

;-- start up --------------------------
fsel   0      40 MHz (T = 25ns)
clk   23 ns
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

wbc     200  ;WBC

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
flush

;--- setup pattern --------------------

pgset 0 b0000  22  ;RES off
pgset 1 b0100 206  ;CAL (=WBC+6)
pgset 2 b0010  16  ;TRG
pgset 3 b0001   0  ;TOK

cald
cal : 0
dtrig
pgsingle
udelay 50
dreadd 2

flush
