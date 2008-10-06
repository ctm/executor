#include "go.h"

#include "keyboard.proto.h"
#include "menu.proto.h"

void
dokeydown (EventRecord * ev)
{
  char ch;
  long val;

  if (ev->modifiers & cmdKey)
    {
      ch = ev->message & charCodeMask;
      val = MenuKey (ch);
      domenu (val);
    }
}
