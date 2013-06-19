daqinit 100000
set 43 1
set 43 2
daqena
set 41 b00101000
dstart
mdelay 10

daqgetptr
daqgetsize
daqrdy

mdelay 500
daqgetsize

mdelay 500
daqgetsize

mdelay 500
daqgetsize
daqrdy

mdelay 500
daqgetsize
daqrdy

daqdis
dstop
dclear
mdelay 10
daqsavebin aaaa.bin
daqdone
flush

set 41 b0100000
