iow $5000402 $54
iow $5000404 b10010000
udelay 200
ior $5000410
udelay 200

iow $5000402 $55
iow $5000404 b01010000
udelay 200
ior $5000410
flush
