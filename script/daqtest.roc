log === startdigroc ===

- RPC_EXPORT void Daq_Select_Datagenerator(uint16_t startvalue);

--- setup readout timing --------------------
pgstop
pgset  0 b001000   4  pg_resr (clear counter)
pgset  1 b100100   4  pg_cal pg_sync (cal = start marker)
pgset  2 b000001   4  pg_tok
pgset  3 b000001   4  pg_tok
pgset  4 b000001   4  pg_tok
pgset  5 b000001   4  pg_tok
pgset  6 b000001   4  pg_tok
pgset  7 b000001   4  pg_tok
pgset  8 b000001   4  pg_tok
pgset  9 b000001   4  pg_tok
pgset 10 b000010   0  pg_trg (trg = end marker)

dopen 10000 0
dselsim 0
dstart

pgsingle
udelay 500
dread

pgset  0 b000000   4

pgsingle
udelay 500
dread

pgsingle
udelay 500
dread

pgsingle
udelay 500
dread

dseloff
dclose

flush
