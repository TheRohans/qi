/*
Terminal Color	RGB Value Used by SGD
Black	        0 0 0
Light red	    255 0 0
Light green	    0 255 0
Yellow	        255 255 0
Light blue	    0 0 255
Light magenta	255 0 255
Light cyan	    0 255 255
High white	    255 255 255
Gray	        128 128 128
Red	            128 0 0
Green	        0 128 0
Brown	        128 128 0
Blue	        0 0 128
Magenta	        128 0 128
Cyan	        0 128 128
White	        192 192 192
*/
    /* root style, must be complete */
    STYLE_DEF(QE_STYLE_DEFAULT, "default",
              QERGB(0xf8, 0xd8, 0xb0), QERGB(0x00, 0x00, 0x00),
              QE_FAMILY_FIXED, 12)

    /* system styles */
    STYLE_DEF(QE_STYLE_MODE_LINE, "mode-line",
              QERGB(0x00, 0x00, 0x00), QERGB(0xe0, 0xe0, 0xe0),
              0, 0)
    STYLE_DEF(QE_STYLE_WINDOW_BORDER, "window-border",
              QERGB(0x00, 0x00, 0x00), QERGB(0xe0, 0xe0, 0xe0),
              0, 0)
    STYLE_DEF(QE_STYLE_MINIBUF, "minibuf",
              QERGB(0xff, 0xff, 0x00), COLOR_TRANSPARENT,
              0, 0)
    STYLE_DEF(QE_STYLE_STATUS, "status",
              QERGB(0xff, 0xff, 0x00), COLOR_TRANSPARENT,
              0, 0)

    /* default style for HTML/CSS2 pages */
    STYLE_DEF(QE_STYLE_CSS_DEFAULT, "css-default",
              QERGB(0x00, 0x00, 0x00), QERGB(0xbb, 0xbb, 0xbb),
              QE_FAMILY_SERIF, 12)

    /* coloring styles */
    STYLE_DEF(QE_STYLE_HIGHLIGHT, "highlight",
              QERGB(0x00, 0x00, 0x00), QERGB(0xff, 0xcc, 0x99),
              0, 0)
    STYLE_DEF(QE_STYLE_SELECTION, "selection",
              QERGB(0xff, 0xff, 0xff), QERGB(0x00, 0x00, 0xff),
              0, 0)
    STYLE_DEF(QE_STYLE_COMMENT, "comment",
              QERGB(0xff, 0x00, 0x00), COLOR_TRANSPARENT,
              0, 0)
    STYLE_DEF(QE_STYLE_PREPROCESS, "preprocess",
              QERGB(0x00, 0x00, 0xff), COLOR_TRANSPARENT,
              0, 0)
    STYLE_DEF(QE_STYLE_STRING, "string",
              QERGB(0x00, 0xff, 0x00), COLOR_TRANSPARENT,
              0, 0)
    STYLE_DEF(QE_STYLE_KEYWORD, "keyword",
              QERGB(0x00, 0xff, 0xff), COLOR_TRANSPARENT,
              0, 0)
    STYLE_DEF(QE_STYLE_FUNCTION, "function",
              QERGB(0xff, 0xff, 0xff), COLOR_TRANSPARENT,
              0, 0)
    STYLE_DEF(QE_STYLE_VARIABLE, "variable",
              QERGB(0xc0, 0xc0, 0xc0), COLOR_TRANSPARENT,
              0, 0)
    STYLE_DEF(QE_STYLE_TYPE, "type",
              QERGB(0x80, 0x80, 0x00), COLOR_TRANSPARENT,
              0, 0)
    STYLE_DEF(QE_STYLE_TAG, "tag",
              QERGB(0x00, 0xff, 0x00), COLOR_TRANSPARENT,
              0, 0)
    STYLE_DEF(QE_STYLE_CSS, "css",
              QERGB(0x00, 0xff, 0xff), COLOR_TRANSPARENT,
              0, 0)
