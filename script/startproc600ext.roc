log === start PROC600 ===

clksrc 1
mdelay 100

resoff

--- set voltages ----------------------------
vd 2650 mV
id  200 mA
va 1500 mV
ia  200 mA

--- setup timing & levels -------------------
clk  6
sda 21  (CLK + 15)
ctr  6  (CLK + 0)
tin 11  (CLK + 5)

clklvl 10
ctrlvl 10
sdalvl 10
tinlvl 10

rocaddr 0
select 0

--- power on --------------------------------
pon
mdelay 500


getid
mdelay 20
getia

--- program ROC -----------------------------
dac   1  10  Vdig 
dac   2  70  Vana
dac   3   8  *NEW* Iph (4bit) (prev Vsf 30)
dac   4  12  Vcomp

dac   7 150  VwllPr
dac   9 150  VwllPr
- dac  10 117  VhldDel
dac  11  40  Vtrim
dac  12  80  VthrComp

dac  13 100  *NEW* VColor (prev VIBias_Bus 30)
- dac  22  99  VIColOr

dac  17 170  *NEW* VoffsetRO (prev 170)

dac  19  50  Vcomp_ADC
dac  20  60  VIref_ADC

dac  25  70  Vcal
dac  26  70  CalDel 24@20MHz

dac  $fe 36  WBC 34
dac  $fd b01_101  CtrlReg (high cal range, StopAcq, TriggerDisable)
flush

mask

d1  9   scope trigger (CH1)
d2 12   tout  (CH2)
a1 1    sdata (CH3)
a2 6    tout  (CH4)

mdelay 200
getid
mdelay 20
getia

--- reset ROC -------------------------------
pgstop
pgset 0 b001000   0  reset
pgsingle
mdelay 20

--- setup readout timing --------------------
pgset 0 b001000  15  pg_resr
pgset 1 b000100  38  pg_cal
pgset 2 b000010  16  pg_trg
pgset 3 b100001   0  pg_tok pg_sync

--- enable pixel ----------------------------
cald
cole :

pixe 10  0 0
pixe 11 30 0
cal  10  0
cal  11 30

flush

--- read out --------------------------------
-dselroc 5
-dopen 1000
-dstart
-pgsingle
-udelay 200
-dstop
-dread
-dclose

udelay 100
-multiread 50

-pgloop 10000
-log *** cluster test
-mdelay 5000
-multiread 30

-pgset 0 b001000 15 pg_resr
-mdelay 500
-pgset 0 b000000 15

-mdelay 500
-multiread 30
pgloop 10000
flush
