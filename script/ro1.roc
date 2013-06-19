stop
trc 50
tct 25   24@40MHz, 21@20MHz, 20@10MHz
ttk 30
wbc 21
cc   1

pixe : 0:1 0
dclear
mdelay 100

cald
cal 0 0 0
udelay 100
seq b0110
run
udelay 500

cald
cal 2 0 0
udelay 100
seq b0110
run
udelay 500

cald
cal 4 0 0
udelay 100
seq b0110
run
udelay 500

dtrig
seq b0001
run
udelay 500
dreadd

dtrig
seq b0001
run
udelay 500
dreadd

dtrig
seq b0001
run
udelay 500
dreadd

flush