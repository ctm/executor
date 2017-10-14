#if !defined (__rsys_syserr_h__)
#  define __rsys_syserr_h__

namespace Executor {











// ### Struct needs manual conversion to GUEST<...>
//   void	(*func8)(void)	PACKED;











// ### Struct needs manual conversion to GUEST<...>
//   void	(*func8)(void)	PACKED;
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


struct adef : GuestStruct {
    GUEST< INTEGER> id;
    GUEST< INTEGER> alen;
    GUEST< INTEGER> primetextid;
    GUEST< INTEGER> secondtextid;
    GUEST< INTEGER> iconid;
    GUEST< INTEGER> procid;
    GUEST< INTEGER> buttonid;
};

struct tdef : GuestStruct {
    GUEST< INTEGER> id;
    GUEST< INTEGER> alen;
    GUEST< Point> loc;
    GUEST< char[1]> text;    /* at least one NUL byte */
};

struct idef : GuestStruct {
    GUEST< INTEGER> id;
    GUEST< INTEGER> alen;
    GUEST< Rect> loc;
    GUEST< LONGINT[32]> ike;
};

    /* NOTE:  THIS IS NOT THE WAY IT WORKS IN THE M*C */
// ### Struct needs manual conversion to GUEST<...>
//   void (*proc)();
    /* NOTE:  THIS IS NOT THE WAY IT WORKS IN THE M*C */
// ### Struct needs manual conversion to GUEST<...>
//   void (*proc)();
struct PACKED pdef {
  INTEGER id;
  INTEGER alen;
  void (*proc)();
  /* NOTE:  THIS IS NOT THE WAY IT WORKS IN THE M*C */
};

// ### Struct needs manual conversion to GUEST<...>

// ### Struct needs manual conversion to GUEST<...>
// struct PACKED bdef {
//   struct PACKED but {
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

struct sdef : GuestStruct {
    GUEST< INTEGER> id;
    GUEST< INTEGER> alen;
    GUEST< char[1]> text;
};
}

#endif /* !defined (__rsys_syserr_h__) */
