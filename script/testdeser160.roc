resoff
clk 10
tin 15

pgset 0 b101000  10  pg_resr pg_sync
pgset 1 b000100  30  pg_cal
pgset 2 b000010  20  pg_trg
pgset 3 b000001 100  pg_tok
pgset 4 b000000   0

deser b1101

d1 9
a1 1
probeadc 1

mdelay 100

dopen 200

dclear
dstart
pgsingle
pgsingle
mdelay(10);
dstop
dread

dclose
flush
