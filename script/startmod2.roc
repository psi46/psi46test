log === startdigmod ===

--- set voltages ----------------------------
vd 2600 mV
id  800 mA
va 1600 mV
ia  800 mA

--- setup timing & levels -------------------
clk  6
sda 21  (CLK + 15)
ctr  6  (CLK + 0)
tin 11  (CLK + 5)

clklvl 10
ctrlvl 10
sdalvl 10
tinlvl 10

--- power on --------------------------------
pon
mdelay 400
resoff
mdelay 200

getid
getia

--- setup TBM -------------------------------
modsel b11111

tbmset $E4 $F0    Init TBM, Reset ROC
tbmset $F4 $F0
tbmset $E0 $81    Disable PKAM Counter, Disable Auto Reset
tbmset $F0 $81
tbmset $E2 $C0    Mode = Calibration
tbmset $F2 $C0
tbmset $E8 $10    Set PKAM Counter
tbmset $F8 $10
tbmset $EA b1_1_101_101 Delays: TOK, Hdr/Trl, Port 1, Port 0
tbmset $FA b1_1_101_101 Delays: TOK, Hdr/Trl, Port 1, Port 0
tbmset $EC $00    Temp measurement control
tbmset $FC $00
tbmset $EE b001_001_00 160/400 MHz phase adjust

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

--- enable pixel ----------------------------
select 0:7
cole :
pixe 1 : 0
cal 1 1

--- setup probe outputs ---------------------
d1 9  sync
ds2 0 4
a1 1  sdata


--- setup readout timing --------------------
pgstop
pgset 0 b010000  20  pg_rest
pgset 1 b000000   0
pgsingle

pgset 0 b000000  15  pg_rest
pgset 1 b000100  30  pg_cal
pgset 2 b100010   5  pg_trg pg_sync
pgset 3 b000000   5  pg_trg
pgset 4 b000000   0  pg_trg


getia
getid

dselmod
dopen 10000 0
dstart

pgsingle
udelay 1000
dread

dseloff
dclose

dsena 0

pgloop 500

flush
