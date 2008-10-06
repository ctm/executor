#include "go.h"
#include "display.h"
#include <assert.h>

#include "initicons.proto.h"

icontableentry **icontable[ICONTABLESIZE], g_foldericons, g_diskicons;
icontableentry *g_foldericonsptr, *g_diskiconsptr;
applist **sigowners[TEXTEDITORPOS + 1];

/* NOTE: the default doc must be the last item.  Its type should really be special
   cased, but it's the default anyway so if a document has a goofy filetype
   it will get the correct information regardless.
 */

typeinfo typearray[] =
{
  'ffil', FONTBAND, NOACTION, FONTICONNUM, 0,
  'FFIL', FONTBAND, NOACTION, FONTSCICONNUM, 0,
  'tfil', FONTBAND, NOACTION, TTFONTICONNUM, 0,
  'dfil', DABAND, OPENDA, DAICONNUM, 0,
  'DFIL', DABAND, OPENDA, DASCICONNUM, 0,
  'APPL', APPBAND, LAUNCH, DEFAULTAPPLICONNUM, 0,
  '|!@$', DOCBAND, LAUNCHCREATOR, DEFAULTDOCICONNUM, 0,
};

ControlHandle
getnewiconcontrol (WindowPtr wp, char **path, long parid,
		   short vrefnum, Str255 s)
{
  item **h;
  Rect r;
  ControlHandle c;

  SetRect (&r, 0, 0, ICONWIDTHUSED, ICONHEIGHTUSED);
  c = NewControl (wp, &r, s, false, 0, 0, 0, ICONCONTROL, 0);
  h = (item **) NewHandle (sizeof (item));
  (*h)->path = path;
  (*h)->ioparid = parid;
  (*h)->vrefnum = vrefnum;
  (*h)->selected = false;
  (*h)->view = ICONVIEW;
  (*c)->contrlRfCon = 0L;
  (*c)->contrlData = (Handle) h;
#ifdef THINK_C
  (*c)->contrlAction = (ProcPtr) -1L;
#else
  (*c)->contrlAction = (ControlActionUPP) -1L;
#endif
  return c;
}

#define NOTFOUND	-1
short
localidlookup (short localid, iconentry ** iconarray, short n)
{
  short i;

  for (i = 0; i < n; i++)
    if ((*iconarray)[i].localid == localid)
/*-->*/ return i;
  return NOTFOUND;
/* TODO: ALERT */
}

void
dobundle (short *spacemade, iconentry *** iconarray, char **h)
{
/* todo: get rid of the *h += n stuff and make a datatype */
  short k, pos, numentries;
  ResType type;
  identry *idarray;
  Handle h2;

  type = *(OSType *) *h;
  *h += 4;
  numentries = *(short *) *h + 1;
  *h += 2;
  if (*spacemade == 0)
    {
      *iconarray = (iconentry **) NewHandle (numentries * sizeof (iconentry));
      *spacemade = numentries;
      idarray = (identry *) *h;
      if (type == 'ICN#')
	{
	  for (k = 0; k < numentries; k++)
	    {
	      (**iconarray)[k].resid = idarray[k].resid;
	      (**iconarray)[k].localid = idarray[k].localid;
	    }
	}
      else if (type == 'FREF')
	{
	  for (k = 0; k < numentries; k++)
	    {
	      h2 = (Handle) Get1Resource ('FREF', idarray[k].resid);
	      (**iconarray)[k].type = *(OSType *) *h2;
	      (**iconarray)[k].localid = *(short *) (*h2 + 4);
	      ReleaseResource (h2);
	    }
	}
      else
	{
/* there shouldn't be any other types */
	  assert (0);
	  /* ALERT */
	}
      *h += numentries * sizeof (identry);
    }
  else
    {
#if 0
/* todo: find out if this assert can ever fail */
/* It can, but the code should work anyways.  This should be looked into further. */
      assert (*spacemade == numentries);
#endif /* 0 */
      idarray = (identry *) * h;
      if (type == 'ICN#')
	{
	  for (k = 0; k < numentries; k++)
	    {
	      pos = localidlookup (idarray[k].localid, *iconarray, numentries);
	      if (pos != NOTFOUND)
		(**iconarray)[pos].resid = idarray[k].resid;
	    }
	}
      else if (type == 'FREF')
	{
	  for (k = 0; k < numentries; k++)
	    {
/*
 * It is my understanding the first two bytes mean nothing.  IMVI says they must be
 * unique, but doesn't use them, but keeps them in for backwards compatibility.  They
 * are local fref ids
 */
	      h2 = (Handle) Get1Resource ('FREF', idarray[k].resid);
	      pos = localidlookup (*(short *) (*h2 + 4), *iconarray, numentries);
	      if (pos != NOTFOUND)
		(**iconarray)[pos].type = *(ResType *) * h2;
	      ReleaseResource (h2);
	    }
	}
      else
/* there shouldn't be any other types */
	assert (0);
      *h += numentries * sizeof (identry);
    }
}
#undef NOTFOUND

typeinfo *
gettypeinfo (OSType type)
{
  short i;

  for (i = 0; i < NELEM (typearray); i++)
    if (type == typearray[i].filetype)
      return &typearray[i];
  return &typearray[NELEM (typearray) - 1];
}

void
geticons (icontableentry ** node, short id)
{
  Handle h;

#define geticonhandle(field, type)		\
	h = (Handle) Get1Resource(type, id);	\
	HandToHand(&h);				\
	(*node)->field = h;

  geticonhandle (icnsh, 'ICN#');
  geticonhandle (icssh, 'ics#');
  geticonhandle (icl8, 'icl8');
  geticonhandle (icl4, 'icl4');
  geticonhandle (ics8, 'ics8');
  geticonhandle (ics4, 'ics4');

#undef	geticonhandle
}

void
inithash (void)
{
  short i;
  icontableentry **node;

  g_foldericonsptr = &g_foldericons;
  g_foldericons.sig = (OSType) 0;
  g_foldericons.type = (OSType) 0;
  g_foldericons.next = 0;
  geticons (&g_foldericonsptr, FOLDERICONNUM);

  g_diskiconsptr = &g_diskicons;
  g_diskicons.sig = (OSType) 0;
  g_diskicons.type = (OSType) 0;
  g_diskicons.next = 0;
  geticons (&g_diskiconsptr, DISKICONNUM);

  for (i = 0; i < ICONTABLESIZE; i++)
    icontable[i] = 0;
  for (i = 0; i < NELEM (typearray); i++)
    {
      node = (icontableentry **) NewHandle (sizeof (icontableentry));
      (*node)->sig = (OSType) 0;
      (*node)->type = typearray[i].filetype;
      (*node)->next = 0;
      geticons (node, typearray[i].iconid);
      typearray[i].iconh = node;
    }
  for (i = 0; i < SIGARRAYSIZE; i++)
    sigowners[i] = 0;
}

icontableentry **
gethash (OSType type, OSType creator)
{
  unsigned short hashval;
  icontableentry **np;

  hashval = ((unsigned long) type + (unsigned long) creator) % ICONTABLESIZE;
  for (np = icontable[hashval]; np != 0; np = (*np)->next)
    {
      if (((*np)->type == type) && ((*np)->sig == creator))
	return np;
    }

  return gettypeinfo (type)->iconh;
}

void
hashicons (ControlHandle c)
{
/* todo: worry about filenames in FREFs */
  /* get FREF, BNDL, ICN#, ics#, icl8, ics8, icl4, ics4 */
  /* BNDL is:
     4 bytes of signature
     2 bytes of 0 [signature resource id?]
     2 bytes of <number of main entries - 1> (almost always 1)
     numberofmainentries *
     4 bytes of 'ICN#' or 'FREF'
     2 bytes of <number of entries - 1>
     numberofentries *
     2 bytes of localid
     2 bytes of resid
   */
/* todo: see what happens with multiple BNDLs */

  char *p;
  short i, j, spacemade, nummain, numbndls;
  unsigned short hashval;
  iconentry **iconarray;
  icontableentry **node;
  Handle h;
  OSType signature;
  short refnum, sigid;
  unsigned char state;
  applist **ah;

  refnum = openappres (c);
  numbndls = Count1Resources ('BNDL');
  for (i = 1; i <= numbndls; i++)
    {
      h = (Handle) Get1IndResource ('BNDL', i);
      state = HGetState (h);
      HLock (h);
      p = *h;
      signature = *(OSType *) p;
      p += 4;
      sigid = *(short *) p;
      p += 2;

      hashval = (unsigned long) signature % SIGARRAYSIZE;
#define CONTROLPROBLEMS
#ifndef CONTROLPROBLEMS
      for (ah = sigowners[hashval]; ah != 0 && (*ah)->sig != signature; ah = (*ah)->next)
	;
      if (ah == 0)
	{
#endif /* CONTROLPROBLEMS */
	  ah = sigowners[hashval];
	  sigowners[hashval] = (applist **) NewHandle (sizeof (applist));
	  (*sigowners[hashval])->next = ah;
	  (*sigowners[hashval])->parid = (*(item **) (*c)->contrlData)->ioparid;
	  (*sigowners[hashval])->vrefnum = (*(item **) (*c)->contrlData)->vrefnum;
	  (*sigowners[hashval])->sig = signature;
	  mystr255copy ((*sigowners[hashval])->name, (*c)->contrlTitle);
#ifndef CONTROLPROBLEMS
	}
#endif /* CONTROLPROBLEMS */

/* todo: find out if nummain must == 2 */
      nummain = *(short *) p + 1;
      p += 2;

      spacemade = 0;
      for (j = 0; j < nummain; j++)
	{
	  dobundle (&spacemade, &iconarray, &p);
	}

      for (j = 0; j < spacemade; j++)
	{
	  hashval = ((unsigned long) signature + (unsigned long) (*iconarray)[j].type)
	    % ICONTABLESIZE;
	  for (node = icontable[hashval]; node != 0 &&
	       ((*node)->sig != signature || (*node)->type != (*iconarray)[j].type);
	       node = (*node)->next)
	    ;
	  if (node == 0)
	    {
	      node = (icontableentry **) NewHandle (sizeof (icontableentry));
	      (*node)->sig = signature;
	      (*node)->type = (*iconarray)[j].type;
	      (*node)->next = icontable[hashval];
	      geticons (node, (*iconarray)[j].resid);
	      icontable[hashval] = node;
	    }
	}
      DisposHandle ((Handle) iconarray);
      HSetState (h, state);
      ReleaseResource (h);
    }
  CloseResFile (refnum);
}

void
setoneicon (ControlHandle c)
{
  CInfoPBRec pb;
  item **ih;
  typeinfo *tip;

  ih = (item **) (*c)->contrlData;
  pb.hFileInfo.ioFDirIndex = 0;
  pb.hFileInfo.ioVRefNum = (*ih)->vrefnum;
  pb.hFileInfo.ioDirID = (*ih)->ioparid;
  pb.hFileInfo.ioNamePtr = (*c)->contrlTitle;
  PBGetCatInfo (&pb, false);
  if (pb.hFileInfo.ioFlAttrib & DIRBIT)
    {
      (*ih)->iconfam = &g_foldericonsptr;
      (*ih)->action = OPENDIR;
      (*ih)->size = 0;
      (*ih)->moddate = pb.dirInfo.ioDrMdDat;
    }
  else
    {
      (*ih)->iconfam = gethash (pb.hFileInfo.ioFlFndrInfo.fdType,
				pb.hFileInfo.ioFlFndrInfo.fdCreator);
      tip = gettypeinfo (pb.hFileInfo.ioFlFndrInfo.fdType);
      (*ih)->action = tip->action;
      (*ih)->size = pb.hFileInfo.ioFlLgLen;
      (*ih)->moddate = pb.hFileInfo.ioFlMdDat;
    }
}

void
seticons (void)
{
  bandinfo *b;
  short i, j;

  b = &bands[VOLBAND];
  for (i = 0; i < b->numitems; i++)
    (*(item **) (*(**b->items)[i])->contrlData)->iconfam = &g_diskiconsptr;
  b = &bands[FOLDERBAND];
  for (i = 0; i < b->numitems; i++)
    (*(item **) (*(**b->items)[i])->contrlData)->iconfam = &g_foldericonsptr;
  for (i = 0; i < NUMBANDS; i++)
    if (i != VOLBAND && i != FOLDERBAND)
      {
	b = &bands[i];
	for (j = 0; j < b->numitems; j++)
	  setoneicon ((**b->items)[j]);
      }
}

short
openappres (ControlHandle c)
{
/* TODO: check error conditions */
  WDPBRec wdpb;
  short refnum;
  short resloadval;

  wdpb.ioVRefNum = (*(item **) (*c)->contrlData)->vrefnum;
  wdpb.ioWDDirID = (*(item **) (*c)->contrlData)->ioparid;
  wdpb.ioWDProcID = 0;
  wdpb.ioNamePtr = 0;
  PBOpenWD (&wdpb, false);
#ifdef THINK_C
  resloadval = ResLoad;
#else
  resloadval = LMGetResLoad ();
#endif
  SetResLoad (false);
  refnum = OpenRFPerm ((*c)->contrlTitle, wdpb.ioVRefNum, fsRdPerm);
  SetResLoad (resloadval);
  PBCloseWD (&wdpb, false);
  if (refnum == -1)
    {
      /* todo: alert */
    }
  return refnum;
}
