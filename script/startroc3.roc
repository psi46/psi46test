log === startdigroc ===

resoff

--- set voltages ----------------------------
vd 2500 mV
id  400 mA
va 1500 mV
ia  400 mA

--- setup timing & levels -------------------
clk  2
sda 17  (CLK + 15)
ctr  2  (CLK + 0)
tin  7  (CLK + 5)

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
dac   1   8  Vdig 
dac   2 100  Vana
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

dac  19  50  Vcomp_ADC 100
dac  20  90  VIref_ADC 160

dac  25  70  Vcal
dac  26  68  CalDel

dac  $fe 50  WBC
dac  $fd  4  CtrlReg
flush

mask

d1 11
a1 1
a2 0

pgstop

--- setup readout timing --------------------
pgset 0 b101000  15  pg_resr pg_sync
pgset 1 b000100  56  pg_cal
pgset 2 b000010  16  pg_trg
pgset 3 b000001   0  pg_tok

--- enable pixel ----------------------------

cald

dselroc 4
mdelay 10

cole :
pixe :10 5 0
cal  :10 5

analyze 70

flush
