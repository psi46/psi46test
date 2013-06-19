orbit   20000  orbit period
set  45  200   orbit gap
set  46  150   reset position

setdel 8 0 0   clock delay
setdel 9 0 0   data delay

set  21  b11   enable trigger and orbit
mdelay  100
set  47  b011  clear event timer, reset TBM
flush
