ia 1000
id 1000
mdelay 100
pon
mdelay 500

clock40
sda   17  17
mdelay 100
tbmmod
flush

set 41 b010010
set 43 b01
mdelay 100
d0l -600
d1l -600
flush
