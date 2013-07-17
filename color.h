// color.h

#ifndef COLOR_H
#define COLOR_H


struct CColor {	float r, g, b; };


#define SIZE_OF_COLORTABLE 28

extern const CColor COLOR_TABLE[SIZE_OF_COLORTABLE];

extern const int COLOR_BIN[];

extern const int COLOR_FAIL[];

extern const int COLOR_CLASS[];

extern const int COLOR_PIC_GOOD[];

extern const int COLOR_PIC_CL2;

extern const int COLOR_PIC_BAD[];


#endif
