typedef enum {
    handle_nil,
    handle_nil_master,
    handle_zero_length,
    handle_valid_unlocked,
    handle_valid_locked,
    handle_NELEM
} handle_state;

typedef enum {
    offset_negative,
    offset_zero
        offset_less_than_handle_size,
    offset_handle_size,
    offset_greater_than_handle_size,
    offset_NELEM
} offset_state;

typedef enum {
    ptr_nil,
    ptr_valid_0,
    ptr_valid_1,
    ptr_valid_2,
    ptr_NELEM
} ptr_state;

typedef enum {
    len_negative,
    len_zero,
    len_valid,
    len_NELEM
} len_state;

static Handle
handle_from_c_string(char *str)
{
    int len;
    Handle retval;

    len = strlen(str);
    retval = NewHandle(len);

    BlockMove(str, *retval, len);

    return retval;
}

static Handle
new_handle(handle_state h_s)
{
    Handle retval;

    switch(h_s)
    {
        case handle_nil:
            retval = 0;
            break;
        case handle_nil_master:
            retval = NewEmptyHandle();
            break;
        case handle_zero_length:
            retval = NewHandle(0);
            break;
        case handle_valid_unlocked:
        case handle_valid_locked:
            retval = handle_from_c_string("I don't repeat once.  I say twice twice.");
            if(h_s == handle_valid_locked)
                HLock(retval);
            break;
    }

    return retval;
}

static LONGINT
new_offset(offset_state o_s, LONGINT hs)
{
    LONGINT retval;
    static int count = 0;

    ++count;
    switch(retval)
    {
        case offset_negative:
            retval = -count;
            break;
        case offset_zero:
            retval = 0;
            break;
        case offset_less_than_handle_size:
            retval = hs - 1 - (count % (hs - 1));
            break;
        case offset_handle_size:
            retval = hs;
            break;
        case offset_greater_than_handle_size:
            retval = hs + count;
            break;
    }

    return retval;
}

static Ptr
new_ptr(ptr_state p_s)
{
    Ptr retval;

    switch(p_s)
    {
        case ptr_nil:
            retval = 0;
            break;
        case ptr_valid_0:
            retval = "nonce";
            break;
        case ptr_valid_1:
            retval = "once";
            break;
        case ptr_valid_2:
            retval = "twice";
            break;
    }

    return retval;
}

static LONGINT
new_len(len_state l_s, LONGINT ps)
{
    LONGINT retval;

    switch(l_s)
    {
        case len_negative:
            retval = -5;
            break;
        case len_zero:
            retval = 0;
            break;
        case len_valid:
            retval = ps;
            break;
    }

    return retval;
}

static void
dump_handle(Handle h)
{
    SignedByte state;
    LONGINT length;

    state = HGetState(h);
    length = GetHandleSize(h);
    HLock(h);

    printf("state = 0x%x, length = %d, bytes = \"%.*s\"\n",
           state, length, (int)length, *h);

    HSetState(h, state);
}

static LONGINT
Munger_invoker(Handle h, LONGINT offset, Ptr ptr1, LONGINT len1,
               Ptr ptr2, LONGINT len2, LONGINT *d0p)
{
    LONGINT retval;
    LONGINT d0_var;

    retval = Munger(h, ptr1, len1, ptr2, len2);
    asm { move.l d0, d0_var }

    *d0p = d0_var;

    return retval;
}

static void
munger_test(handle_state h_s,
            offset_state offset_s,
            ptr_state ptr1_s,
            len_state len1_s,
            ptr_state ptr2_s,
            len_state len2_s)
{
    Handle h;
    LONGINT offset;
    Ptr ptr1;
    LONGINT len1;
    Ptr ptr2;
    LONGINT len2;
    LONGINT retval;
    LONGINT d0;

    h = new_handle(h_s);
    offset = new_offset(offset_s, GetHandleSize(h));
    ptr1 = new_ptr(ptr1_s);
    len1 = new_len(len1_s);
    ptr2 = new_ptr(ptr2_s);
    len2 = new_len(len2_s);

    dump_handle(h);
    retval = Munger_invoker(h, offset, ptr1, len1, ptr2, len2, &d0);
    dump_handle(h);
    dump_retval_d0(retval, d0);

    if(h_s >= handle_nil_master)
        DisposeHandle(h);
}

static void
munger_test_all(void)
{
    handle_state h_s;
    offset_state offset_s;
    ptr_state ptr1_s;
    len_state len1_s;
    ptr_state ptr2_s;
    len_state len2_s;

    for(h_s = 0; h_s < handle_NELEM; ++h_s)
        for(offset_s = 0; offset_s < offset_NELEM; ++offset_s)
            for(ptr1_s = 0; ptr1_s < ptr_NELEM; ++ptr1_s)
                for(len1_s = 0; len1_s < len_NELEM; ++len1_s)
                    for(ptr2_s = 0; ptr2_s < ptr_NELEM; ++len2_s)
                        for(len2_s = 0; len2_s < len_NELEM; ++len2_s)
                            munger_test(h_s, offset_s, ptr1_s, len1_s, ptr2_s, len2_s);
}
