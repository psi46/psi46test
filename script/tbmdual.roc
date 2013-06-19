modsel b11111
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
tbmset $EE $c0    Analog Output DAC Gain

mdelay 100
