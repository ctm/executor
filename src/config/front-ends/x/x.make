
FRONT_END_DEFINES =

X_SRC = x.c x_keycodes.c

# x front end supplies an image view server
HAVE_IV = yes

FRONT_END_SRC = $(X_SRC)
FRONT_END_OBJ = $(FRONT_END_SRC:.c=.o)

FRONT_END_LIBS = @xlibdir@ -lXext -lX11

clean::
	rm -f $(FRONT_END_OBJ)
