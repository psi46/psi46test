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

tbmset $E4 b11110000    Init TBM, Reset ROC
tbmset $F4 b11110000

tbmset $E0 b1_000000_1  Disable Auto Reset, Disable PKAM Counter
tbmset $F0 b1_000000_1

tbmset $E2 b11_000000   Mode: Calibration
tbmset $F2 b11_000000

tbmset $E8 0            Set PKAM Counter (x+1)*6.4us
tbmset $F8 16

tbmset $EA b1_1_100_100 Delays: Tok _ Hdr/Trl _ Port 1 _ Port 0
tbmset $FA b1_1_100_100

tbmset $EC 10           Auto reset rate (x+1)*256
tbmset $FC 0

tbmset $EE b001_001_00  160/400 MHz phase adjust  
tbmset $FE $00          Temp measurement control

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

dac  25 180  Vcal
dac  26  50  CalDel

dac  $fe 24  WBC
dac  $fd  4  CtrlReg
flush

mask
cald

mdelay 100
getid
getia

--- setup probe outputs ---------------------
d1  9  sync
d2  0  
a1  1  sdata1
lcds

--- setup readout timing --------------------
pgstop
pgset 0 b010000  50  pg_rest
pgset 1 b001000   0  pg_resr
pgsingle
udelay 100

pgset 0 b000000  15  pg_rest
pgset 1 b000100  30  pg_cal
pgset 2 b100010  30  pg_trg pg_sync
pgset 3 b000100  30  pg_cal
pgset 4 b000010  30  pg_trg
pgset 5 b000100  30  pg_cal
pgset 6 b000010   0  pg_trg

- pgloop 20000

select :
cole :
- pixe : 10:25 0

select 1
pixe 1:4 8 0
cal  1:4 8

select 5
pixe 1:3 10:11 0
cal  1:3 10:11

flush
