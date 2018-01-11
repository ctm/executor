#if !defined(__rsys_syserr_h__)
#define __rsys_syserr_h__

namespace Executor
{

struct myalerttab_t
{
    GUEST_STRUCT;
    GUEST<INTEGER> count;

    GUEST<INTEGER> id1;
    GUEST<INTEGER> len1;
    GUEST<INTEGER> prim1;
    GUEST<INTEGER> sec1;
    GUEST<INTEGER> icon1;
    GUEST<INTEGER> proc1;
    GUEST<INTEGER> but1;

    GUEST<INTEGER> id2;
    GUEST<INTEGER> len2;
    GUEST<Point> point2;
    GUEST<unsigned char[52]> str2;

    GUEST<INTEGER> id3;
    GUEST<INTEGER> len3;

    GUEST<Rect> rect3;
    GUEST<unsigned char[32][4]> icon3;

    GUEST<INTEGER> id4;
    GUEST<INTEGER> len4;

    GUEST<Point> point4;
    GUEST<char[46]> str4;

    GUEST<INTEGER> id5;
    GUEST<INTEGER> len5;
    GUEST<INTEGER> num5;
    GUEST<INTEGER> str5;
    GUEST<Rect> rect5;
    GUEST<INTEGER> proc5;
    GUEST<INTEGER> str5b;
    GUEST<Rect> rect5b;
    GUEST<INTEGER> proc5b;

    GUEST<INTEGER> id6;
    GUEST<INTEGER> len6;

    GUEST<char[4]> str6;

    GUEST<INTEGER> id7;
    GUEST<INTEGER> len7;
    GUEST<char[6]> str7;

    GUEST<INTEGER> id8;
    GUEST<INTEGER> len8;
    GUEST<void (*)(void)> func8;
};

struct adef
{
    GUEST_STRUCT;
    GUEST<INTEGER> id;
    GUEST<INTEGER> alen;
    GUEST<INTEGER> primetextid;
    GUEST<INTEGER> secondtextid;
    GUEST<INTEGER> iconid;
    GUEST<INTEGER> procid;
    GUEST<INTEGER> buttonid;
};

struct tdef
{
    GUEST_STRUCT;
    GUEST<INTEGER> id;
    GUEST<INTEGER> alen;
    GUEST<Point> loc;
    GUEST<char[1]> text; /* at least one NUL byte */
};

struct idef
{
    GUEST_STRUCT;
    GUEST<INTEGER> id;
    GUEST<INTEGER> alen;
    GUEST<Rect> loc;
    GUEST<LONGINT[32]> ike;
};

struct pdef
{
    GUEST_STRUCT;
    GUEST<INTEGER> id;
    GUEST<INTEGER> alen;
    GUEST<void (*)()> proc;
    /* NOTE:  THIS IS NOT THE WAY IT WORKS IN THE M*C */
};

struct bdef
{
    GUEST_STRUCT;
    GUEST<INTEGER> id;
    GUEST<INTEGER> alen;
    GUEST<INTEGER> nbut;
    struct but
    {
        GUEST<INTEGER> butstrid;
        GUEST<Rect> butloc;
        GUEST<INTEGER> butprocid;
    } buts[1];
};

struct sdef
{
    GUEST_STRUCT;
    GUEST<INTEGER> id;
    GUEST<INTEGER> alen;
    GUEST<char[1]> text;
};
}

#endif /* !defined (__rsys_syserr_h__) */
