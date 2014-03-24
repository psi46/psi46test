log === startdigroc ===

resoff

--- set voltages ----------------------------
vd 2500 mV
id  400 mA
va 1500 mV
ia  400 mA

--- setup timing & levels -------------------
clk  4
sda 19  (CLK + 15)
ctr  4  (CLK + 0)
tin  9  (CLK + 5)

clklvl 10
ctrlvl 10
sdalvl 10
tinlvl 10

rocaddr 0
select 0

--- power on --------------------------------
pon
mdelay 500

--- program ROC -----------------------------
dac   1   4  Vdig 
dac   2 130  Vana
dac   3  30  Vsf
dac   4  12  Vcomp

dac   7 150  VwllPr
dac   9 150  VwllPr
dac  10 117  VhldDel
dac  11  40  Vtrim
dac  12  80  VthrComp

dac  13  30  VIBias_Bus
dac  22  99  VIColOr

dac  17 170  VoffsetRO

dac  19  50  Vcomp_ADC
dac  20  90  VIref_ADC

dac  25  70  Vcal
dac  26  68  CalDel

dac  $fe 14  WBC
dac  $fd  4  CtrlReg
flush

mask

d1 9
a1 1

--- setup readout timing --------------------
pgstop
pgset 0 b001000  15  pg_resr pg_sync
pgset 1 b000100  20  pg_cal
pgset 2 b000010  16  pg_trg
pgset 3 b100001   0  pg_tok

--- enable pixel ----------------------------
cald

cole :
pixe 2:4 2 0
cal  2:4 2

dopen 1000000 0
dselroc 4
dstart

pgsingle
udelay 200
dread

pgsingle
udelay 200
dread

dstop
dseloff
dselroc 4
dstart

pgsingle
udelay 200
dread

pgsingle
udelay 200
dread

dstop
dread
dclose

pgloop 10000

flush
