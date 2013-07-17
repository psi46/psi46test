resoff
vd 2500
pon

rocaddr 0
select 0
mdelay 500

clk  4
sda 19  (CLK + 15)
ctr  4  (CLK + 0)
tin  9  (CLK + 5)

clklvl 10
ctrlvl 10
sdalvl 10
tinlvl 10

rocdigdefault

pgstop
pgset 0 b100000   5  pg_sync
pgset 1 b001000  10  pg_resr
pgset 2 b000100  10  pg_cal
pgset 3 b000010   5  pg_trg
pgset 4 b000001   0  pg_tok

d1 9
d2 17

udelay(20)

probeadc 3
adcena 50

dopen 20000
dstart
pgsingle
udelay(500)
dstop
dreada 100
dclose
mdelay(100);

flush
