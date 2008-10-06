
/* window.c */
void setwindowicons (CWindowPeek wp);
CWindowPtr createdirwindow (CInfoPBRec * dir, Rect * r, char **path, short volume);
void dodragwin (Point start, WindowPtr wp);
void dogrowwin (Point start, WindowPtr wp);
void disposedirwindow (WindowPtr wp);
void dogoaway (Point start, WindowPtr wp);
