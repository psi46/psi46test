id 600
ia 600
pon
mdelay 500
clock40
rda 3  3@40MHz(+/-5),22@20MHz(+/-11),88@10MHz(+/-24),38@5MHz(+/-49),138@2.5MHz(+/-99)
mdelay 100
modtbm
flush

dlevel -500

mask
vthr  60
vcal  60

trc 20
tct 24   24@40MHz, 21@20MHz, 20@10MHz
ttk 10
wbc 21
cc   1
trep 10

set 41 b0100010
seq b1111
loop
mdelay 100
cole 0
cole 1
pixe 0 0 0
cal 0 0
flush
