log === startdigroc ===

resoff

--- set voltages ----------------------------
vd 2500 mV
id  400 mA
va 1500 mV
ia  400 mA

--- setup timing & levels -------------------
clk 16
sda 31  (CLK + 15)
ctr 16  (CLK + 0)
tin 21  (CLK + 5)

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
dac   2 145  Vana
dac   3  30  Vsf
dac   4  12  Vcomp
dac   5   0  Vleak_comp
dac   6   0  VrgPr
dac   7  35  VwllPr
dac   8   0  VrgSh
dac   9  35  VwllPr
dac  10 160  VhldDel
dac  11   7  Vtrim
dac  12  70  VthrComp

dac  13  30  VIBias_Bus
dac  14  10  Vbias_sf
dac  15  50  VoffsetOp
dac  16 115  VIbiasOp
dac  17 120  VoffsetRO
dac  18 115  VIon
dac  19 220  VIbias_PH
dac  20 153  Ibias_DAC
dac  21 220  VIbias_roc
dac  22  99  VIColOr
dac  23   0  Vnpix
dac  24   0  VsumCol
dac  25  50  Vcal
dac  26  30  CalDel

dac  $fe 50  WBC
dac  $fd  4  CtrlReg
flush

mask
cald

d1 11
a1 1
a2 0

pgstop

--- setup readout timing --------------------
pgset 0 b101000  15  pg_resr pg_sync
pgset 1 b000100  54  pg_cal
pgset 2 b000010  16  pg_trg
pgset 3 b000001   0  pg_tok

--- enable pixel ----------------------------

mdelay 10

cole :
pixe :1 5 0
cal  :1 5

analyzeana 70

flush
