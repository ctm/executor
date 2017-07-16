#if !defined (__rsys_syserr_h__)
#  define __rsys_syserr_h__

namespace Executor {
typedef struct PACKED {
  INTEGER count;

  INTEGER id1;
  INTEGER len1;
  INTEGER prim1;
  INTEGER sec1;
  INTEGER icon1;
  INTEGER proc1;
  INTEGER but1;

  INTEGER id2;
  INTEGER len2;
  Point	point2;
  unsigned char str2[52];

  INTEGER id3;
  INTEGER len3;

  Rect rect3;
  unsigned char icon3[32][4];

  INTEGER id4;
  INTEGER len4;

  Point	point4;
  char	str4[46];

  INTEGER id5;
  INTEGER len5;
  INTEGER num5;
  INTEGER str5;
  Rect	rect5;
  INTEGER proc5;
  INTEGER str5b;
  Rect	rect5b;
  INTEGER	proc5b	PACKED;

  INTEGER id6;
  INTEGER len6;

  char	str6[4];

  INTEGER id7;
  INTEGER len7;
  char	str7[6];

  INTEGER id8;
  INTEGER len8;
  void	(*func8)(void)	PACKED;
} myalerttab_t;


struct PACKED adef {
  INTEGER id;
  INTEGER alen;
  INTEGER primetextid;
  INTEGER secondtextid;
  INTEGER iconid;
  INTEGER procid;
  INTEGER buttonid;
};

struct PACKED tdef {
  INTEGER id;
  INTEGER alen;
  Point loc;
  char text[1];	/* at least one NUL byte */
};

struct PACKED idef {
  INTEGER id;
  INTEGER alen;
  Rect loc;
  LONGINT ike[32];
};

struct PACKED pdef {
  INTEGER id;
  INTEGER alen;
  void (*proc)();
  /* NOTE:  THIS IS NOT THE WAY IT WORKS IN THE M*C */
};

struct PACKED bdef {
  INTEGER id;
  INTEGER  alen;
  INTEGER  nbut;
  struct PACKED but {
    INTEGER butstrid;
    Rect butloc;
    INTEGER butprocid;
  } buts[1];
};

struct PACKED sdef {
  INTEGER id;
  INTEGER alen;
  char text[1];
};
}

#endif /* !defined (__rsys_syserr_h__) */
