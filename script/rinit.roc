pgdis
probe 1 6
mdelay 10

pgset 0 b1000  50

pgset 1 b0100  26
pgset 2 b0010 100

pgset 3 b0100  26
pgset 4 b0010   0

pgset 5 b0001 100
pgset 6 b0001   0

flush
