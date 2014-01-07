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

dac  $fe 15  WBC
dac  $fd  4  CtrlReg
flush

mask

--- setup readout timing --------------------
pgstop
pgset 0 b101000  15  pg_resr pg_sync
pgset 1 b000100  20  pg_cal
pgset 2 b000010  16  pg_trg
pgset 3 b000001   0  pg_tok

--- enable pixel ----------------------------
cald

flush
