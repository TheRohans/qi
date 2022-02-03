#ifndef QE_COLOR_H
#define QE_COLOR_H

typedef unsigned int QEColor;
#define QEARGB(a,r,g,b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define QERGB(r,g,b) QEARGB(0xff, r, g, b)
#define COLOR_TRANSPARENT 0
#define QECOLOR_XOR       1

#endif
