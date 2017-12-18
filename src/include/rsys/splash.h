#if !defined(_SPLASH_H_)
#define _SPLASH_H_

struct splash_screen_rect
{
    int16_t top __attribute__((packed));
    int16_t left __attribute__((packed));
    int16_t bottom __attribute__((packed));
    int16_t right __attribute__((packed));
};

struct splash_screen_header
{
    uint32_t bpp __attribute__((packed));
    uint32_t log2_bpp __attribute__((packed));
    uint32_t color_count __attribute__((packed));

    uint32_t color_offset __attribute__((packed));
    uint32_t splash_bits_offset __attribute__((packed));
    uint32_t button_bits_offset __attribute__((packed));

    uint32_t button_row_bytes __attribute__((packed));

    uint16_t button_height __attribute__((packed));

    int16_t button_y __attribute__((packed));
    int16_t button_x_byte __attribute__((packed));

    uint16_t n_buttons __attribute__((packed));

    /* gcc 4.3.0 will complain about the packed attributes below
     so they're commented out below.  Most likely they were
     never needed and should be completely excised from the
     code. */

    struct splash_screen_rect button_rects[4]
        /* __attribute__ ((packed)) */;

    uint8 bg_pixel /* __attribute__ ((packed)) */;
    uint8 dummy_1[3] /* __attribute__ ((packed)) */;
};

struct splash_screen_color
{
    int16_t dummy_1 __attribute__((packed));

    /* must be in big endian byte order */
    int16_t red __attribute__((packed));
    int16_t green __attribute__((packed));
    int16_t blue __attribute__((packed));
};

#define SPLASH_SCREEN_HEIGHT 480
#define SPLASH_SCREEN_WIDTH 640

#if defined(DISPLAY_SPLASH_SCREEN)

extern bool splash_screen_display(bool button_p, char *basename);
extern void splash_screen_event_loop(void);

#endif /* DISPLAY_SPLASH_SCREEN */

#endif /* _SPLASH_H_ */
