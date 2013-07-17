// color.cpp


#include "color.h"


const CColor COLOR_TABLE[SIZE_OF_COLORTABLE] =
{
	{ 0.0, 0.0, 0.0 },  //  0 black
	{ 0.5, 0.5, 0.5 },  //  1 dark gray
	{ .75, .75, .75 },  //  2 light gray
	{ 1.0, 1.0, 1.0 },  //  3 white

	{ .75, 0.0, 0.0 },  //  4 dark red
	{ 1.0, 0.0, 0.0 },  //  5 red
	{ 1.0, 0.4, 0.5 },  //  6 light red

	{ 0.0, 0.5, 0.0 },  //  7 dark green
	{ 0.0, .75, 0.0 },  //  8 green
	{ 0.0, 1.0, 0.0 },  //  9 green
	{ .75, 1.0, 0.0 },  // 10 yellow green
	{ 0.7, 1.0, 0.7 },  // 11 light green

	{ 0.0, 0.5, 0.5 },  // 12 dark blue green

	{ 0.0, 0.0, 1.0 },  // 13 dark blue
	{ .25, 0.5, 1.0 },  // 14 blue
	{ 0.0, .75, 1.0 },  // 15 blue
	{ 0.5, .75, 1.0 },  // 16 blue
	{ 0.0, 1.0, 1.0 },  // 17 light blue
	{ 0.5, 1.0, 1.0 },  // 18 light blue

	{ 0.5, .25, 0.0 },  // 19 dark brown
	{ .75, 0.5, 0.0 },  // 20 brown
	{ 1.0, 0.5, .25 },  // 21 orange
	{ 1.0, .75, 0.5 },  // 22 light orange
	{ 1.0, 1.0, 0.0 },  // 23 yellow
	{ 1.0, 1.0, .75 },  // 24 light yellow

	{ 0.5, 0.0, 0.5 },  // 25 dark red blue
	{ 1.0, 0.0, 1.0 },  // 26 red blue
	{ 1.0, .75, 1.0 }   // 27 light red blue
};



const int COLOR_BIN[] =
{
	 8,
	 5,
	23,
	21,
	24,
	25,
	26,
	27,
	12,
	13,
	14,
	16,
	18
};

const int COLOR_FAIL[] =
{ 
	 5,
	23,
	21,
	24,
	25,
	26,
	13,
	14,
	17,
	 0,
	 1,
	 2,
	 3,
	 7,
	19,
	20,
	22,
	  
	12,
	27,
	 4,
	  
	 6,
	11,
	10,
	 8
};

const int COLOR_CLASS[] =
{
	 8,
	10,
	17,
	23,
	 5
};

const int COLOR_PIC_GOOD[] =
{
	 7,
	18,
	 8,
	17,
	 9,
	16,
	10,
	15,
	11,
	14,
	12,
	13
};

const int COLOR_PIC_CL2 = 20;

const int COLOR_PIC_BAD[] =
{
	 5,
	23,
	21,
	 4,
	24,
	 6,
	22,
	 1,
	 2,
	 3
};

