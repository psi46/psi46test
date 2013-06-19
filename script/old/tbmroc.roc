tbmsel 0 0
tbmset $E4 $F0    Init TBM, Reset ROC
tbmset $F4 $F0
tbmset $E0 $01    ReadOutSpeed = full
tbmset $F0 $01
tbmset $E2 $C0    Mode = Pre-Calibration
tbmset $F2 $C0
tbmset $E8 $00    Enable TBM A
tbmset $F8 $00    Enable TBM B

tbmset $EA $c0    Analog Input Amplifier Bias
tbmset $EC $c0    Analog Output Driver Bias
tbmset $EE $80    Analog Output DAC Gain

select 0
rocsettings

ctl  b0100
mask
vcal 180
trc 20
tct 30
ttk 20
wbc 27
cc   1
trep 40

cal 0 0
