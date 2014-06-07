log === startdigmod ===

--- set voltages ----------------------------
vd 2600 mV
id  800 mA
va 1600 mV
ia  800 mA

--- setup timing & levels -------------------
clk  4
sda 19  (CLK + 15)
ctr  4  (CLK + 0)
tin  9  (CLK + 5)

clklvl 10
ctrlvl 10
sdalvl 10
tinlvl 10

--- power on --------------------------------
pon
mdelay 400
resoff
mdelay 200

--- setup TBM -------------------------------
modsel b11111

tbmset $E4 $F0    Init TBM, Reset ROC
tbmset $F4 $F0
tbmset $E0 $01    Disable PKAM Counter
tbmset $F0 $01
tbmset $E2 $C0    Mode = Calibration
tbmset $F2 $C0
tbmset $E8 $10    Set PKAM Counter
tbmset $F8 $10
tbmset $EA b00000000 Delays
tbmset $FA b00000000
tbmset $EC $00    Temp measurement control
tbmset $FC $00

mdelay 100

--- setup all ROCs --------------------------
select :

dac   1   8  Vdig 
dac   2 120  Vana
dac   3  40  Vsf
dac   4  12  Vcomp

dac   7  60  VwllPr
dac   9  60  VwllPr
dac  10 117  VhldDel
dac  11  40  Vtrim
dac  12  20  VthrComp

dac  13  30  VIBias_Bus
dac  14   6  Vbias_sf
dac  22  99  VIColOr

dac  15  40  VoffsetOp
dac  17  80  VoffsetRO
dac  18 115  VIon

dac  19  50  Vcomp_ADC 100
dac  20  70  VIref_ADC 160

dac  25  70  Vcal
dac  26  68  CalDel

dac  $fe 25  WBC
dac  $fd  4  CtrlReg
flush

mask
cald

mdelay 100
getid
getia

--- setup probe outputs ---------------------
d1 9  sync
a1 1  sdata

--- setup readout timing --------------------
pgstop
pgset 0 b010000  20  pg_rest
pgset 1 b000000   0
pgsingle

pgset 0 b010000  15  pg_rest
pgset 1 b000100  30  pg_cal
pgset 2 b100010   0  pg_trg pg_sync
pgset 3 b000100  30  pg_cal
pgset 4 b000010  30  pg_trg
pgset 5 b000100  30  pg_cal
pgset 6 b000010   0  pg_trg

- pgloop 20000

select :

vcal 180
dac 26 50  caldel
wbc 24

cole :
pixe : 10:25 0

select 0
cal  0:20 10
select 1
cal  0:20 11
select 2
cal  0:20 12
select 3
cal  0:20 13
select 4
cal  0:20 14
select 5
cal  0:20 15
select 6
cal  0:20 16
select 7
cal  0:20 17
select 8
cal  0:20 18
select 9
cal  0:20 19
select 10
cal  0:20 20
select 11
cal  0:20 21
select 12
cal  0:20 22
select 13
cal  0:20 23
select 14
cal  0:20 24
select 15
cal  0:20 25


flush
