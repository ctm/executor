#if !defined (__rsys_syserr_h__)
#  define __rsys_syserr_h__

typedef struct {
    INTEGER	count	PACKED;

    INTEGER	id1	PACKED,
		len1	PACKED,
		prim1	PACKED,
		sec1	PACKED,
		icon1	PACKED,
		proc1	PACKED,
		but1	PACKED;

    INTEGER	id2	PACKED,
    		len2	PACKED;
    Point	point2	LPACKED;
    unsigned char str2[52]	LPACKED;

    INTEGER	id3	PACKED,
		len3	PACKED;

    Rect	rect3	LPACKED;
    char	icon3[32][4]	LPACKED;

    INTEGER	id4	PACKED,
		len4	PACKED;

    Point	point4	LPACKED;
    char	str4[46]	LPACKED;

    INTEGER	id5	PACKED,
   	        len5	PACKED,
		num5	PACKED,
		str5	PACKED;
    Rect	rect5	LPACKED;
    INTEGER	proc5	PACKED,
    		str5b	PACKED;
    Rect	rect5b	LPACKED;
    INTEGER	proc5b	PACKED;

    INTEGER	id6	PACKED,
		len6	PACKED;

    char	str6[4]	LPACKED;

    INTEGER	id7	PACKED,
		len7	PACKED;
    char	str7[6]	LPACKED;

    INTEGER	id8	PACKED,
		len8	PACKED;
    void	(*func8)(void)	PACKED;

} myalerttab_t;


struct adef {
    INTEGER id			PACKED,
	    alen		PACKED,
	    primetextid		PACKED,
	    secondtextid	PACKED,
	    iconid		PACKED,
	    procid		PACKED,
	    buttonid		PACKED;
};

struct tdef {
    INTEGER id			PACKED,
	    alen		PACKED;
    Point loc			LPACKED;
    char text[1]		LPACKED;	/* at least one NUL byte */
};

struct idef {
    INTEGER id		PACKED,
	    alen	PACKED;
    Rect loc		LPACKED;
    LONGINT ike[32]	PACKED;
};

struct pdef {
    INTEGER id		PACKED,
	    alen	PACKED;
    void (*proc)()	PACKED;
			/* NOTE:  THIS IS NOT THE WAY IT WORKS IN THE M*C */
};

struct bdef {
    INTEGER id			PACKED,
	    alen		PACKED,
	    nbut		PACKED;
    struct but {
	INTEGER butstrid	PACKED;
	Rect butloc		LPACKED;
	INTEGER butprocid	PACKED;
    } buts[1]			LPACKED;
};

struct sdef {
    INTEGER id		PACKED,
	    alen	PACKED;
    char text[1]	LPACKED;
};

#endif /* !defined (__rsys_syserr_h__) */
