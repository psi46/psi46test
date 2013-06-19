pgdis
probe 1 6
mdelay 10

=== sequence ================

--- reset
pgset 0 b1000  50

--- 1 cal/trig
pgset 1 b0100  26
pgset 2 b0010   4
pgset 3 b0001 200

--- 2. cal/trig
pgset 4 b0100  26
pgset 5 b0010   0

--- run sequence
dtrig
pgsingle
mdelay 10


=== 2 data read out =========
pgset 0 b0001 0

dtrig
pgsingle
udelay 100

dtrig
pgsingle
udelay 100

dreadd

flush
