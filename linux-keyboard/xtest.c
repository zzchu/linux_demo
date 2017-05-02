//1.终端输入xev，可以测试键盘和鼠标
//2, reference code:http://web.mit.edu/nawm/src/nawm/keymap.c
#include <X11/extensions/XTest.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* keymap.c: watches the X keymaps and does clever things with keys */

/* Copyright (C) 1999 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */


Display *dpy;

static KeySym *syms;
static XModifierKeymap *mods;
static int keysyms_per_keycode, min_keycode, max_keycode;

static char *modname[] = { "Shift", "Lock", "Control", "Meta",
    "Mod2", "Mod3", "Mod4", "Mod5" };

bool get_key_state(KeyCode key )
{
    char keys[32];
    XQueryKeymap( dpy, keys );
    
    int byte = key >> 3;
    int bit = key - ( byte << 3 );

    return ((keys[byte] & (1 << bit)) != 0);
}

void initkeymap(void)
{
    XDisplayKeycodes(dpy, &min_keycode, &max_keycode);
    syms = XGetKeyboardMapping(dpy, min_keycode, max_keycode - min_keycode + 1,
                               &keysyms_per_keycode);
    mods = XGetModifierMapping(dpy);
}

void update_keymap(void)
{
    XFree(syms);
    XFreeModifiermap(mods);
    initkeymap();
}

struct modstate {
    KeyCode kc;
    int pressed;
};

void *set_modifier_state(int setmods)
{
    int nmods, modsize, mod, key;
    bool state, wantstate;
    struct modstate *old;
    //char keys[32];
    KeyCode kc;
    
    nmods = 0;
    modsize = 10;
    old = (modstate *)malloc(modsize * sizeof(struct modstate));
    memset(old, 0, modsize * sizeof(struct modstate));
    
    //XQueryKeymap(dpy, keys);
    
    /* For each modifier... */
    for (mod = 0; mod < 8; mod++)
    {
        wantstate = ((setmods & (1 << mod)) != 0);
        
        if ((mod >= 0) && (wantstate == false))  {
            continue;
        }
        /* For each key that generates that modifier... */
        for (key = 0; key < mods->max_keypermod; key++)
        {
            kc = mods->modifiermap[mod * mods->max_keypermod + key];
            if (!kc)
                break;
            state = get_key_state(kc);
            //state = 0;
            
            
            /* If the key isn't in the state we want, fix it and record
             * that we changed it.
             */
            if (state != wantstate)
            {
                if (nmods == modsize - 1)
                {
                    old = (modstate *)realloc(old, modsize * 2 * sizeof(struct modstate));
                    memset(old + modsize * sizeof(struct modstate), 0,
                           modsize * sizeof(struct modstate));
                    modsize *= 2;
                }
                old[nmods].kc = kc;
                old[nmods].pressed = state;
                nmods++;
                
                if (wantstate)
                    XTestFakeKeyEvent(dpy, kc, True, CurrentTime);
                else
                    XTestFakeKeyEvent(dpy, kc, False, CurrentTime);
            }
            
            /* If we want the modifier to be set, we only need to look
             * at the first key, since we'll have set it if it wasn't
             * already set. (If we want it unset, we need to check each
             * key and unset any that are set.)
             */
            if (wantstate)
                break;
        }
    }
    return (void *)old;
}

void reset_modifier_state(void *oldstate)
{
    struct modstate *old = (struct modstate *)oldstate;
    int i;
    
    for (i = 0; old[i].kc; i++)
    {
        if (!old[i].pressed)
            XTestFakeKeyEvent(dpy, old[i].kc, False, CurrentTime);
        else
            XTestFakeKeyEvent(dpy, old[i].kc, True, CurrentTime);
    }
    free(old);
}

void typechar(KeySym ks)
{
    KeyCode kc;
    int mod, xmod = -1, control = 0, start;
    void *old;
    
    if (ks < 32)
    {
        control = ControlMask;
        ks += 96;
    }
    kc = XKeysymToKeycode(dpy, ks);
    start = (kc - min_keycode) * keysyms_per_keycode;
    
    for (mod = 0; mod < 2; mod++)
    {
        /* This sucks, but OpenWindows does this wrong. (It lists
         * the uppercase letter in the 0 position and doesn't list
         * the lowercase letter at all.)
         */
        if (isalpha(ks))
        {
            if (mod == 0 && islower(ks) && tolower(syms[start]) == ks)
            {
                /* Looking at unshifted position, want a lowercase
                 * letter, and this is the letter we're looking for.
                 */
                xmod = 0;
            }
            else if (mod == 1 && isupper(ks) &&
                     (syms[start + mod] == ks || syms[start] == ks))
            {
                /* Looking at shifted position, want an uppercase
                 * letter, and either this is the letter we're looking
                 * for or the unshifted one was.
                 */
                xmod = 1;
            }
        }
        else if (syms[start + mod] == ks)
            xmod = mod;
        
        if (xmod != -1)
        {
            old = set_modifier_state(xmod + control);
            XTestFakeKeyEvent(dpy, kc, True, CurrentTime);
            XTestFakeKeyEvent(dpy, kc, False, CurrentTime);
            reset_modifier_state(old);
            break;
        }
    }
}

int main (int argc,char * argv[]){
    
    //Display *dpy = NULL;
    //XEvent event;
    dpy = XOpenDisplay (NULL);
    update_keymap();
    
    sleep (3);
#if 0

    typechar('A', true);
    typechar('A', false);
    typechar(XK_Shift_L, true);
    typechar(',', true);
    typechar(',', false);
    typechar('.', true);
    typechar('.', false);
    typechar(XK_Shift_L, false);
    //typechar(XK_BackSpace, true);
    //typechar(XK_BackSpace, false);
    //typechar('2', true);
    //typechar('2', false);
    typechar('@', true);
    typechar('@', false);
    typechar('<', true);
    typechar('<', false);
    typechar('>', true);
    typechar('>', false);
    typechar('*', true);
    typechar('*', false);
    typechar('8', true);
    typechar('8', false);
    typechar(XK_Control_L, true);
    typechar('a', true);
    typechar('a', false);
    typechar(XK_Control_L, false);

#else
    
    KeyCode kc = XKeysymToKeycode(dpy, XK_kana_fullstop);
    
    printf("keycode :%d\n", kc);
    
    KeySym lower_return, upper_return;
    XConvertCase('a', &lower_return, &upper_return);
    
    printf("lower:%d, upper:%d\n", lower_return, upper_return);
    
    //XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_kana_fullstop), True, 0);
    //XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_kana_fullstop), False, 0);
    //typechar('<');
#if 0
    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Shift_L), True, 0);
    typechar(',');
    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Shift_L), False, 0);
    

    typechar('A');
    //typechar(XK_Shift_L);
    typechar('2');
    typechar('@');
    typechar('<');
    typechar('>');
    typechar('*');
    typechar('8');
    
    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_kana_fullstop), True, 0);
    typechar('a');
    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), False, 0);
#endif
#endif
    /* Get the current pointer position */
//    XQueryPointer (dpy, RootWindow (dpy, 0), &event.xbutton.root,
//                   &event.xbutton.window, &event.xbutton.x_root,
//                   &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y,
//                   &event.xbutton.state);
    
    /* Fake the pointer movement to new relative position */
    //sleep(3);
    
//    printf("x=>%d, y=%d \n", x, y);
//    XTestFakeMotionEvent (dpy, 0, x, y, 0);
//    XSync(dpy, 0);
//    sleep(1);
//    XTestFakeMotionEvent (dpy, 0, x+2, y+2, 0);
//    XSync(dpy, 0);
//    sleep(1);
//    XTestFakeMotionEvent (dpy, 0, x+5, y+5, 0);
//    XSync(dpy, 0);
//    sleep(1);
//    XSync(dpy, 0);
//    sleep(1);
//    
//    /* Fake the pointer movement to new absolate position */
//    XTestFakeMotionEvent (dpy, 0, 250, 250, CurrentTime);
//    sleep(1);
    
    /* Fake the mouse button Press and Release events */
//    XTestFakeButtonEvent (dpy, 1, True,  0);
//    XTestFakeButtonEvent (dpy, 1, False, 0);
//    XSync(dpy, 0);
//    sleep(1);
    
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Caps_Lock), true, 0);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Caps_Lock), false, 0);
//    XSync(dpy, 0);
    
//    printf("zz_dbg: %d == %x \n", XkbKeysymToModifiers(dpy, XK_W), ShiftMask);
//    printf("zz_dbg: %d == %x \n", XkbKeysymToModifiers(dpy, XK_w), ShiftMask);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Shift_L), true, 0);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_W), true, 0);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Shift_L), false, 0);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_W), false, 0);
    

//    XSync(dpy, 0);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Y), true, 0);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Y), false, 0);
//    XSync(dpy, 0);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Z), true, 0);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Z), false, 0);
//    XSync(dpy, 0);
//    sleep(1);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), True, 0);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), true, 0);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_a), true, 0);
//    
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_a), false, 0);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), false, 0);
//    XSync(dpy, 0);
//    sleep(1);
//    
//    XTestFakeButtonEvent (dpy, 1, True,  0);
//    XTestFakeButtonEvent (dpy, 1, False, 0);
//    XSync(dpy, 0);
//    sleep(1);
//    
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_a), True, 0);
//    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_a), false, 0);
    //sleep(1);
    
    XCloseDisplay (dpy);
    return 0;
}
