/*
 *	Terminal Color	RGB Value Used by SGD
 * ------------------------------
 *	Black	        0 0 0
 *	Light red	    255 0 0
 *	Light green	    0 255 0
 *	Yellow	        255 255 0
 *	Light blue	    0 0 255
 *	Light magenta	255 0 255
 *	Light cyan	    0 255 255
 *	High white	    255 255 255
 * ------------------------------
 *	Gray	        128 128 128
 *	Red	            128 0 0
 *	Green	        0 128 0
 *	Brown	        128 128 0
 *	Blue	        0 0 128
 *	Magenta	        128 0 128
 *	Cyan	        0 128 128
 *	White	        192 192 192
 */
 
    // root style, must be complete
    STYLE_DEF(QE_STYLE_DEFAULT, "default",
              QERGB(255, 255, 255), QERGB(0, 0, 0),
              QE_FAMILY_FIXED, 12, TS_RESET)

    // system styles
    STYLE_DEF(QE_STYLE_MODE_LINE, "mode-line",
              QERGB(0, 0, 0), QERGB(192,192,192),
              0, 0, TS_RESET)
    STYLE_DEF(QE_STYLE_WINDOW_BORDER, "window-border",
              QERGB(0, 0, 0), QERGB(128, 128, 128),
              0, 0, TS_RESET)
    STYLE_DEF(QE_STYLE_MINIBUF, "minibuf",
              QERGB(0xff, 0xff, 0xff), COLOR_TRANSPARENT,
              0, 0, TS_RESET)
    STYLE_DEF(QE_STYLE_STATUS, "status",
              QERGB(0xff, 0xff, 0xff), COLOR_TRANSPARENT,
              0, 0, TS_RESET)

    // generic coloring styles
    STYLE_DEF(QE_STYLE_HIGHLIGHT, "highlight",
              QERGB(0x00, 0x00, 0x00), QERGB(192, 192, 192),
              0, 0, TS_RESET)
    STYLE_DEF(QE_STYLE_SELECTION, "selection",
              QERGB(0, 0, 0), QERGB(192, 192, 192),
              0, 0, TS_RESET)
    STYLE_DEF(QE_STYLE_COMMENT, "comment",
              QERGB(255, 255, 0), COLOR_TRANSPARENT,
              0, 0, TS_RESET)
    STYLE_DEF(QE_STYLE_PREPROCESS, "preprocess",
              QERGB(0, 255, 255), COLOR_TRANSPARENT,
              0, 0, TS_RESET)
    STYLE_DEF(QE_STYLE_STRING, "string",
              QERGB(0, 255, 0), COLOR_TRANSPARENT,
              0, 0, TS_RESET)
    STYLE_DEF(QE_STYLE_KEYWORD, "keyword",
              QERGB(255, 0, 255), COLOR_TRANSPARENT,
              0, 0, TS_RESET)
    STYLE_DEF(QE_STYLE_FUNCTION, "function",
              QERGB(0, 0, 0), QERGB(255, 255, 255), 
              0, 0, TS_RESET)
    STYLE_DEF(QE_STYLE_VARIABLE, "variable",
              QERGB(255, 255, 255), COLOR_TRANSPARENT,
              0, 0, TS_RESET)
    STYLE_DEF(QE_STYLE_TYPE, "type",
              QERGB(0, 255, 255), COLOR_TRANSPARENT,
              0, 0, TS_RESET)

    // mostly html
    STYLE_DEF(QE_STYLE_TAG, "tag",
              QERGB(128, 0, 128), COLOR_TRANSPARENT,
              0, 0, TS_RESET)
    STYLE_DEF(QE_STYLE_CSS, "css",
              QERGB(0x00, 0xff, 0xff), COLOR_TRANSPARENT,
              0, 0, TS_RESET)
