resoff
vd 2500
pon

chipid 0
select 0
mdelay 500

clk  7
sda 22  (CLK + 15)
ctr  7  (CLK + 0)
tin 12  (CLK + 5)

rocdigdefault
vthr  60
vcal  60

dac  20 80  VIref_ADC 160
dac  15 40  VOffsetOp
dac  17 120 VOffsetRO 

pgdis
pgset 0 b001000  20  pg_resr pg_sync
pgset 1 b000100  25  pg_cal
pgset 2 b000010  16  pg_trg
pgset 3 b100001   0  pg_tok

wbc 21

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
cal  : 4 0
cal  : 8 0

mdelay(100);

deser b1101

dopen 2000
dstart
pgsingle
udelay(500)
dstop
dread 2000
dclose

pgloop 1000
flush
