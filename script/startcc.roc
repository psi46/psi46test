resoff
vd 2500
pon

rocaddr 0
select 0
mdelay 500

clk 23
sda 38  (CLK + 15)
ctr 23  (CLK + 0)
tin 28  (CLK + 5)

rocdigdefault
dac 1 0
vthr  60
vcal  60

dac  20 80  VIref_ADC 160
dac  15 40  VOffsetOp
dac  17 120 VOffsetRO 

wbc 81

pgstop
pgset 0 b001000   0  pg_resr
pgsingle
udelay 30
flush

pgstop
pgset 0 b000000  20  pg_resr pg_sync
pgset 1 b000100  87  pg_cal
pgset 2 b000100  85  pg_cal
pgset 3 b000010  20  pg_trg
pgset 4 b100001   0  pg_tok

d1 9   sync
d2 10  tin
a1 1   sdata1
udelay(20)
a2 6   tout
udelay(20)

udelay 100
cole :
pixe : 4 0
pixe : 8 0
cal  : 4 0 cal  : 8 0

mdelay(100);

deser b101

dopen 20000
dstart
pgsingle
udelay(500)
dstop
dread 2000
dclose
mdelay(100);

pgloop 40000
takedata2

flush
