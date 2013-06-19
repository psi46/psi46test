modsel 31
tbmset $E4 $F0    Init TBM, Reset ROC
tbmset $F4 $F0
tbmset $E0 $01    ReadOutSpeed = full
tbmset $F0 $01
tbmset $E2 $C0    Mode = Pre-Calibration
tbmset $F2 $C0
tbmset $E8 $00    Enable TBM A
tbmset $F8 $03    Enable TBM B

tbmset $EA $f0    Analog Input Amplifier Biasf
tbmset $EC $f0    Analog Output Driver Bias
tbmset $EE $80    Analog Output DAC Gain

select 0
rocsettings
select 1
rocsettings
select 2
rocsettings
select 3
rocsettings
select 4
rocsettings
select 5
rocsettings
select 6
rocsettings
select 7
rocsettings
select 8
rocsettings
select 9
rocsettings
select 10
rocsettings
select 11
rocsettings
select 12
rocsettings
select 13
rocsettings
select 14
rocsettings
select 15
rocsettings
select 0

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
