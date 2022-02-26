// color.cpp


#include "color.h"


const CColor COLOR_TABLE[SIZE_OF_COLORTABLE] =
{
	{ 0.0f, 0.0f, 0.0f },  //  0 black
	{ 0.5f, 0.5f, 0.5f },  //  1 dark gray
	{ .75f, .75f, .75f },  //  2 light gray
	{ 1.0f, 1.0f, 1.0f },  //  3 white

	{ .75f, 0.0f, 0.0f },  //  4 dark red
	{ 1.0f, 0.0f, 0.0f },  //  5 red
	{ 1.0f, 0.4f, 0.5f },  //  6 light red

	{ 0.0f, 0.5f, 0.0f },  //  7 dark green
	{ 0.0f, .75f, 0.0f },  //  8 green
	{ 0.0f, 1.0f, 0.0f },  //  9 green
	{ .75f, 1.0f, 0.0f },  // 10 yellow green
	{ 0.7f, 1.0f, 0.7f },  // 11 light green

	{ 0.0f, 0.5f, 0.5f },  // 12 dark blue green

	{ 0.0f, 0.0f, 1.0f },  // 13 dark blue
	{ .25f, 0.5f, 1.0f },  // 14 blue
	{ 0.0f, .75f, 1.0f },  // 15 blue
	{ 0.5f, .75f, 1.0f },  // 16 blue
	{ 0.0f, 1.0f, 1.0f },  // 17 light blue
	{ 0.5f, 1.0f, 1.0f },  // 18 light blue

	{ 0.5f, .25f, 0.0f },  // 19 dark brown
	{ .75f, 0.5f, 0.0f },  // 20 brown
	{ 1.0f, 0.5f, .25f },  // 21 orange
	{ 1.0f, .75f, 0.5f },  // 22 light orange
	{ 1.0f, 1.0f, 0.0f },  // 23 yellow
	{ 1.0f, 1.0f, .75f },  // 24 light yellow

	{ 0.5f, 0.0f, 0.5f },  // 25 dark red blue
	{ 1.0f, 0.0f, 1.0f },  // 26 red blue
	{ 1.0f, .75f, 1.0f }   // 27 light red blue
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

