pgset 0 b100000  10  pg_sync
pgset 1 b000010  19  pg_trg
pgset 2 b001000  19  pg_resr
pgset 3 b010000  19  pg_rest
pgset 4 b000000  19  pg_cal
pgset 5 b000001  0   pg_tok

d1 15
d2 17
a1 7
a2 3
probeadc 3

adcena
mdelay 100

daqstart 512
pgsingle pgloop 1000  pgsingle
mdelay(100);
pgsingle pgloop 1000  pgsingle
mdelay(100);

daqend


adcdis
flush
