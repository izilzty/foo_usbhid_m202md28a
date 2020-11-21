#pragma once

#include "stdafx.h"

extern cfg_int cfg_usbhid_vid;
extern cfg_int cfg_usbhid_pid;

extern cfg_int cfg_onplay_power;
extern cfg_int cfg_onpause_power;
extern cfg_int cfg_onstop_power;
extern cfg_int cfg_onexit_power;

extern cfg_string cfg_onplay_format1;
extern cfg_string cfg_onplay_format2;
extern cfg_string cfg_onload_string1;
extern cfg_string cfg_onload_string2;
extern cfg_string cfg_onstop_string1;
extern cfg_string cfg_onstop_string2;

extern cfg_int cfg_spectrum_enable_onplay;
extern cfg_int cfg_spectrum_enable_onstop;
extern cfg_int cfg_spectrum_fft_speed;
extern cfg_int cfg_spectrum_draw_speed;
extern cfg_int cfg_spectrum_pos_x;
extern cfg_int cfg_spectrum_len_x;
extern cfg_int cfg_spectrum_enable_fake;

extern cfg_int cfg_onvolume_hold;
extern cfg_string cfg_onvolume_string2;

extern cfg_int cfg_fadein_speed;
extern cfg_int cfg_scroll_wait;
extern cfg_int cfg_scroll_speed;

extern bool reset_FROM;
extern bool write_FROM;
