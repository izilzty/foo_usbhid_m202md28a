#include "preferences.h"
#include "resource.h"

static const char* page_name = "HID VFD Display";

/* 默认数值 */

static const uint16_t default_vid = 0x1008;
static const uint16_t default_pid = 0x101A;

static const uint8_t default_onplay_power = 3;
static const uint8_t default_onpause_power = 3;
static const uint8_t default_onstop_power = 3;
static const uint8_t default_onexit_power = 5;

static const char default_onplay_format1[] = u8"《[%title%]》 $if(%album artist%,[%album artist%],未知艺术家)     ";
static const char default_onplay_format2[] = u8"$padcut(%codec% $pad_right($div(%samplerate%,1000),2,0).$div($sub(%samplerate%,$mul($div(%samplerate%,1000),1000)),100)k,14, ) $padcut_right($ifgreater($div(%playback_time_seconds%,60),99,99,$div(%playback_time_seconds%,60)),2,0):$ifgreater($div(%playback_time_seconds%,60),99,++,$padcut_right($sub(%playback_time_seconds%,$mul($div(%playback_time_seconds%,60),60)),2,0))";
static const char default_onload_string1[] = u8"";
static const char default_onload_string2[] = u8"";
static const char default_onstop_string1[] = u8"播放已停止。";
static const char default_onstop_string2[] = u8"---- --.-k     --:--";

static const uint8_t default_spectrum_enable_onplay = 0;
static const uint8_t default_spectrum_enable_onstop = 0;
static const uint16_t default_spectrum_fft_speed = 40;
static const uint16_t default_spectrum_draw_speed = 20;
static const uint16_t default_spectrum_pos_x = 0;
static const uint16_t default_spectrum_len_x = 20;

static const uint16_t default_onvolume_hold = 2000;
static const char default_onvolume_string2[] = u8"     音量:";

static const uint16_t default_fadein_speed = 15;
static const uint16_t default_scroll_wait = 3000;
static const uint16_t default_scroll_speed = 300;

/* GUID标识 */

static const GUID guid_page = { 0xcbc50983, 0xbc4d, 0x4968, { 0xa5, 0x83, 0xb6, 0x75, 0xf9, 0x78, 0x4a, 0x58 } };

static const GUID guid_cfg_vid = { 0x379c77f4, 0x4fd4, 0x46cb, { 0xb4, 0x1d, 0xd3, 0x8d, 0x59, 0x7e, 0xea, 0xd6 } };
static const GUID guid_cfg_pid = { 0x7054f428, 0xadfc, 0x4da0, { 0x94, 0x82, 0xfa, 0xe2, 0x91, 0xa8, 0x92, 0x70 } };

static const GUID guid_cfg_onplay_power = { 0xe97b3d3, 0x4ea0, 0x4dec, { 0xb5, 0xc3, 0x26, 0x3, 0xec, 0x3a, 0xc0, 0x51 } };
static const GUID guid_cfg_onpause_power = { 0xce0ee58f, 0x9e1d, 0x41fb, { 0x9e, 0xba, 0x95, 0xff, 0xea, 0xae, 0xa6, 0xe1 } };
static const GUID guid_cfg_onstop_power = { 0xfc2e18ad, 0xde25, 0x4afb, { 0xaf, 0x39, 0x46, 0xc4, 0x9d, 0x1d, 0xab, 0x8 } };
static const GUID guid_cfg_onexit_power = { 0x4d93972c, 0xa51a, 0x4315, { 0xb4, 0x86, 0xd4, 0x9d, 0xcb, 0x1, 0x43, 0x2b } };

static const GUID guid_cfg_onplay_format1 = { 0xab3c6647, 0x1eeb, 0x4c5f, { 0x9d, 0xa7, 0xfe, 0x9, 0x7c, 0xdc, 0x50, 0x51 } };
static const GUID guid_cfg_onplay_format2 = { 0x5afb8365, 0xe798, 0x4f94, { 0x9d, 0x8c, 0x46, 0x6f, 0xff, 0xcb, 0x9a, 0xdc } };
static const GUID guid_cfg_onload_string1 = { 0x26a69654, 0x6bf7, 0x4be9, { 0xb1, 0x98, 0x5b, 0xdf, 0x5a, 0x3b, 0x8b, 0x8a } };
static const GUID guid_cfg_onload_string2 = { 0xdfaf0466, 0x9bdf, 0x4062, { 0xb8, 0xaf, 0x43, 0xc3, 0xb1, 0x9b, 0x22, 0x42 } };
static const GUID guid_cfg_onstop_string1 = { 0x47ba1435, 0xe2f, 0x4021, { 0x87, 0x33, 0x1c, 0x5d, 0x7f, 0xa0, 0x2a, 0x1b } };
static const GUID guid_cfg_onstop_string2 = { 0xb8468f81, 0xa91b, 0x4e8b, { 0xaa, 0x9b, 0x62, 0xcc, 0x38, 0xe9, 0xb7, 0x4c } };

static const GUID guid_cfg_spectrum_enable_onplay = { 0xcddf0207, 0x2dd2, 0x456a, { 0x98, 0xd2, 0x3b, 0x3c, 0xb2, 0xbb, 0xb7, 0x16 } };
static const GUID guid_cfg_spectrum_enable_onstop = { 0x96c38b4b, 0x4a8e, 0x490d, { 0x85, 0x2e, 0x64, 0x79, 0xb4, 0x3, 0xdf, 0xac } };
static const GUID guid_cfg_spectrum_fft_speed = { 0xeecb2bbe, 0xea95, 0x4e39, { 0xb3, 0x71, 0xef, 0x5a, 0xc, 0x6d, 0x12, 0x66 } };
static const GUID guid_cfg_spectrum_draw_speed = { 0x24d1f78b, 0x4d36, 0x4050, { 0x88, 0x44, 0x92, 0xf9, 0x47, 0x7a, 0x25, 0x15 } };
static const GUID guid_cfg_spectrum_pos_x = { 0xbb3b6447, 0x7ddd, 0x43ed, { 0xad, 0x2f, 0x3e, 0x2c, 0x59, 0xc7, 0xb8, 0xbd } };
static const GUID guid_cfg_spectrum_len_x = { 0xea22b571, 0x7aae, 0x40e2, { 0x95, 0xbc, 0xc, 0x18, 0x9f, 0xd8, 0x91, 0x1a } };

static const GUID guid_cfg_onvolume_style = { 0x810e6738, 0xe6cd, 0x4a4e, { 0xbf, 0x51, 0xdc, 0x40, 0x7b, 0x7f, 0x40, 0x63 } };
static const GUID guid_cfg_onvolume_string2 = { 0xf9c4c639, 0x13dd, 0x4fb0, { 0xbf, 0x1f, 0xf9, 0xc9, 0x79, 0x60, 0x2b, 0x41 } };

static const GUID guid_cfg_fadein_speed = { 0xa3729235, 0xad83, 0x4527, { 0xac, 0x50, 0xf1, 0xff, 0xb2, 0x9b, 0xec, 0x17 } };
static const GUID guid_cfg_scroll_wait = { 0x5e17bc4, 0x6143, 0x4aa1, { 0x9a, 0x34, 0xed, 0xf8, 0x43, 0x97, 0xae, 0x2b } };
static const GUID guid_cfg_scroll_speed = { 0x47149fa6, 0x93a0, 0x4209, { 0x9e, 0x1e, 0xf7, 0x8e, 0xcd, 0x11, 0x7d, 0x74 } };

/* 存储初始化 */

cfg_int cfg_usbhid_vid(guid_cfg_vid, default_vid);
cfg_int cfg_usbhid_pid(guid_cfg_pid, default_pid);

cfg_int cfg_onplay_power(guid_cfg_onplay_power, default_onplay_power);
cfg_int cfg_onpause_power(guid_cfg_onpause_power, default_onpause_power);
cfg_int cfg_onstop_power(guid_cfg_onstop_power, default_onstop_power);
cfg_int cfg_onexit_power(guid_cfg_onexit_power, default_onexit_power);

cfg_string cfg_onplay_format1(guid_cfg_onplay_format1, default_onplay_format1);
cfg_string cfg_onplay_format2(guid_cfg_onplay_format2, default_onplay_format2);
cfg_string cfg_onload_string1(guid_cfg_onload_string1, default_onload_string1);
cfg_string cfg_onload_string2(guid_cfg_onload_string2, default_onload_string2);
cfg_string cfg_onstop_string1(guid_cfg_onstop_string1, default_onstop_string1);
cfg_string cfg_onstop_string2(guid_cfg_onstop_string2, default_onstop_string2);

cfg_int cfg_spectrum_enable_onplay(guid_cfg_spectrum_enable_onplay, default_spectrum_enable_onplay);
cfg_int cfg_spectrum_enable_onstop(guid_cfg_spectrum_enable_onstop, default_spectrum_enable_onstop);
cfg_int cfg_spectrum_fft_speed(guid_cfg_spectrum_fft_speed, default_spectrum_fft_speed);
cfg_int cfg_spectrum_draw_speed(guid_cfg_spectrum_draw_speed, default_spectrum_draw_speed);
cfg_int cfg_spectrum_pos_x(guid_cfg_spectrum_pos_x, default_spectrum_pos_x);
cfg_int cfg_spectrum_len_x(guid_cfg_spectrum_len_x, default_spectrum_len_x);

cfg_int cfg_onvolume_hold(guid_cfg_onvolume_style, default_onvolume_hold);
cfg_string cfg_onvolume_string2(guid_cfg_onvolume_string2, default_onvolume_string2);

cfg_int cfg_fadein_speed(guid_cfg_fadein_speed, default_fadein_speed);
cfg_int cfg_scroll_wait(guid_cfg_scroll_wait, default_scroll_wait);
cfg_int cfg_scroll_speed(guid_cfg_scroll_speed, default_scroll_speed);

bool reset_FROM;
bool write_FROM;

static uint32_t str_to_cfgint(LPCTSTR str)
{
    uint32_t value = 0;

    _stscanf_s(str, _T("%x"), &value);
    return value;
}

static void cfgint_to_str(uint32_t cfg, LPTSTR str, uint8_t str_len)
{
    _sntprintf_s(str, str_len, str_len - 1, _T("0x%04X"), cfg);
}

class CPreferences : public CDialogImpl<CPreferences>, public preferences_page_instance
{
public:
    //Constructor - invoked by preferences_page_impl helpers - don't do Create() in here, preferences_page_impl does this for us
    CPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}

    //Note that we don't bother doing anything regarding destruction of our class.
    //The host ensures that our dialog is destroyed first, then the last reference to our preferences_page_instance object is released, causing our object to be deleted.

    //dialog resource ID
    enum { IDD = IDD_PREFERENCES };
    t_uint32 get_state();
    void apply();
    void reset();

    //WTL message map
    BEGIN_MSG_MAP(CPreferences)
        MSG_WM_INITDIALOG(OnInitDialog)

        COMMAND_HANDLER_EX(IDC_TX_VID, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_TX_PID, EN_CHANGE, OnEditChange)

        COMMAND_HANDLER_EX(IDC_CB_POWER_PLAY, CBN_SELCHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_CB_POWER_PAUSE, CBN_SELCHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_CB_POWER_STOP, CBN_SELCHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_CB_POWER_EXIT, CBN_SELCHANGE, OnEditChange)

        COMMAND_HANDLER_EX(IDC_TX_PLAY_FORMAT1, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_TX_PLAY_FORMAT2, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_TX_LOAD_STRING1, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_TX_LOAD_STRING2, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_TX_STOP_STRING1, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_TX_STOP_STRING2, EN_CHANGE, OnEditChange)

        COMMAND_HANDLER_EX(IDC_TX_SPFFTSPEED_STRING, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_TX_SPDRAWSPEED_STRING, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_TX_SPPOSX_STRING, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_TX_SPLENX_STRING, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_CK_SPEN_ONPLAY, BN_CLICKED, OnEditChange)
        COMMAND_HANDLER_EX(IDC_CK_SPEN_ONSTOP, BN_CLICKED, OnEditChange)

        COMMAND_HANDLER_EX(IDC_TX_VOLUME_HOLD_STRING, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_TX_VOLUME_STRING2, EN_CHANGE, OnEditChange)

        COMMAND_HANDLER_EX(IDC_TX_FADEIN_STRING, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_TX_SCROLL_WAIT_STRING, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_TX_SCROLL_STRING, EN_CHANGE, OnEditChange)

        COMMAND_HANDLER_EX(IDC_BT_FROM_RESET, BN_CLICKED, reset_from)
        COMMAND_HANDLER_EX(IDC_BT_FROM_WRITE, BN_CLICKED, write_from)
    END_MSG_MAP()
private:
    BOOL OnInitDialog(CWindow, LPARAM);
    void OnEditChange(UINT, int, CWindow);
    bool HasChanged();
    void OnChanged();
    void reset_from(UINT, int, CWindow);
    void write_from(UINT, int, CWindow);

    const preferences_page_callback::ptr m_callback;
    bool value_change;
};

BOOL CPreferences::OnInitDialog(CWindow, LPARAM)
{
    const uint8_t str_len = 6 + 1;
    LPTSTR str_vid = new TCHAR[str_len];
    LPTSTR str_pid = new TCHAR[str_len];

    cfgint_to_str(cfg_usbhid_vid, str_vid, str_len);
    cfgint_to_str(cfg_usbhid_pid, str_pid, str_len);

    SetDlgItemText(IDC_TX_VID, str_vid);
    SetDlgItemText(IDC_TX_PID, str_pid);

    delete[]str_vid;
    delete[]str_pid;

    ((CComboBox)GetDlgItem(IDC_CB_POWER_PLAY)).AddString(_T("Dim 25%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PLAY)).AddString(_T("Dim 50%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PLAY)).AddString(_T("Dim 75%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PLAY)).AddString(_T("Dim 100%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PLAY)).AddString(_T("Dim OFF"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PLAY)).AddString(_T("Power OFF"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PLAY)).SetCurSel(cfg_onplay_power);

    ((CComboBox)GetDlgItem(IDC_CB_POWER_PAUSE)).AddString(_T("Dim 25%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PAUSE)).AddString(_T("Dim 50%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PAUSE)).AddString(_T("Dim 75%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PAUSE)).AddString(_T("Dim 100%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PAUSE)).AddString(_T("Dim OFF"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PAUSE)).AddString(_T("Power OFF"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PAUSE)).SetCurSel(cfg_onpause_power);

    ((CComboBox)GetDlgItem(IDC_CB_POWER_STOP)).AddString(_T("Dim 25%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_STOP)).AddString(_T("Dim 50%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_STOP)).AddString(_T("Dim 75%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_STOP)).AddString(_T("Dim 100%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_STOP)).AddString(_T("Dim OFF"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_STOP)).AddString(_T("Power OFF"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_STOP)).SetCurSel(cfg_onstop_power);

    ((CComboBox)GetDlgItem(IDC_CB_POWER_EXIT)).AddString(_T("Dim 25%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_EXIT)).AddString(_T("Dim 50%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_EXIT)).AddString(_T("Dim 75%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_EXIT)).AddString(_T("Dim 100%"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_EXIT)).AddString(_T("Dim OFF"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_EXIT)).AddString(_T("Power OFF"));
    ((CComboBox)GetDlgItem(IDC_CB_POWER_EXIT)).SetCurSel(cfg_onexit_power);

    uSetDlgItemText(*this, IDC_TX_PLAY_FORMAT1, cfg_onplay_format1);
    uSetDlgItemText(*this, IDC_TX_PLAY_FORMAT2, cfg_onplay_format2);
    uSetDlgItemText(*this, IDC_TX_LOAD_STRING1, cfg_onload_string1);
    uSetDlgItemText(*this, IDC_TX_LOAD_STRING2, cfg_onload_string2);
    uSetDlgItemText(*this, IDC_TX_STOP_STRING1, cfg_onstop_string1);
    uSetDlgItemText(*this, IDC_TX_STOP_STRING2, cfg_onstop_string2);

    ((CCheckBox)GetDlgItem(IDC_CK_SPEN_ONPLAY)).SetCheck(cfg_spectrum_enable_onplay);
    ((CCheckBox)GetDlgItem(IDC_CK_SPEN_ONSTOP)).SetCheck(cfg_spectrum_enable_onstop);
    SetDlgItemInt(IDC_TX_SPFFTSPEED_STRING, cfg_spectrum_fft_speed, false);
    SetDlgItemInt(IDC_TX_SPDRAWSPEED_STRING, cfg_spectrum_draw_speed, false);
    SetDlgItemInt(IDC_TX_SPPOSX_STRING, cfg_spectrum_pos_x, false);
    SetDlgItemInt(IDC_TX_SPLENX_STRING, cfg_spectrum_len_x, false);

    uSetDlgItemText(*this, IDC_TX_VOLUME_STRING2, cfg_onvolume_string2);
    SetDlgItemInt(IDC_TX_VOLUME_HOLD_STRING, cfg_onvolume_hold, false);

    SetDlgItemInt(IDC_TX_FADEIN_STRING, cfg_fadein_speed, false);
    SetDlgItemInt(IDC_TX_SCROLL_WAIT_STRING, cfg_scroll_wait, false);
    SetDlgItemInt(IDC_TX_SCROLL_STRING, cfg_scroll_speed, false);

    reset_FROM = false;
    write_FROM = false;

    value_change = false;

    return FALSE;
}

void CPreferences::OnEditChange(UINT, int, CWindow)
{
    // not much to do here
    value_change = true;
    OnChanged();
}

t_uint32 CPreferences::get_state()
{
    t_uint32 state = preferences_state::resettable;
    if (HasChanged()) state |= preferences_state::changed;
    return state;
}

void CPreferences::reset()
{
    const uint8_t str_len = 6 + 1;
    LPTSTR str_vid = new TCHAR[str_len];
    LPTSTR str_pid = new TCHAR[str_len];

    cfgint_to_str(default_vid, str_vid, str_len);
    cfgint_to_str(default_pid, str_pid, str_len);

    SetDlgItemText(IDC_TX_VID, str_vid);
    SetDlgItemText(IDC_TX_PID, str_pid);

    delete[]str_vid;
    delete[]str_pid;

    ((CComboBox)GetDlgItem(IDC_CB_POWER_PLAY)).SetCurSel(default_onplay_power);
    ((CComboBox)GetDlgItem(IDC_CB_POWER_PAUSE)).SetCurSel(default_onpause_power);
    ((CComboBox)GetDlgItem(IDC_CB_POWER_STOP)).SetCurSel(default_onstop_power);
    ((CComboBox)GetDlgItem(IDC_CB_POWER_EXIT)).SetCurSel(default_onexit_power);

    uSetDlgItemText(*this, IDC_TX_PLAY_FORMAT1, default_onplay_format1);
    uSetDlgItemText(*this, IDC_TX_PLAY_FORMAT2, default_onplay_format2);
    uSetDlgItemText(*this, IDC_TX_LOAD_STRING1, default_onload_string1);
    uSetDlgItemText(*this, IDC_TX_LOAD_STRING2, default_onload_string2);
    uSetDlgItemText(*this, IDC_TX_STOP_STRING1, default_onstop_string1);
    uSetDlgItemText(*this, IDC_TX_STOP_STRING2, default_onstop_string2);

    ((CCheckBox)GetDlgItem(IDC_CK_SPEN_ONPLAY)).SetCheck(default_spectrum_enable_onplay);
    ((CCheckBox)GetDlgItem(IDC_CK_SPEN_ONSTOP)).SetCheck(default_spectrum_enable_onstop);
    SetDlgItemInt(IDC_TX_SPFFTSPEED_STRING, default_spectrum_fft_speed, false);
    SetDlgItemInt(IDC_TX_SPDRAWSPEED_STRING, default_spectrum_draw_speed, false);
    SetDlgItemInt(IDC_TX_SPPOSX_STRING, default_spectrum_pos_x, false);
    SetDlgItemInt(IDC_TX_SPLENX_STRING, default_spectrum_len_x, false);

    uSetDlgItemText(*this, IDC_TX_VOLUME_STRING2, default_onvolume_string2);
    SetDlgItemInt(IDC_TX_VOLUME_HOLD_STRING, default_onvolume_hold, false);

    SetDlgItemInt(IDC_TX_FADEIN_STRING, default_fadein_speed, false);
    SetDlgItemInt(IDC_TX_SCROLL_WAIT_STRING, default_scroll_wait, false);
    SetDlgItemInt(IDC_TX_SCROLL_STRING, default_scroll_speed, false);

    OnChanged();
}

void CPreferences::apply()
{
    const uint8_t str_len = 6 + 1;
    LPTSTR str_vid = new TCHAR[str_len];
    LPTSTR str_pid = new TCHAR[str_len];

    GetDlgItemText(IDC_TX_VID, str_vid, str_len);
    GetDlgItemText(IDC_TX_PID, str_pid, str_len);

    cfg_usbhid_vid = str_to_cfgint(str_vid);
    cfg_usbhid_pid = str_to_cfgint(str_pid);

    delete[]str_vid;
    delete[]str_pid;

    cfg_onplay_power = ((CComboBox)GetDlgItem(IDC_CB_POWER_PLAY)).GetCurSel();
    cfg_onpause_power = ((CComboBox)GetDlgItem(IDC_CB_POWER_PAUSE)).GetCurSel();
    cfg_onstop_power = ((CComboBox)GetDlgItem(IDC_CB_POWER_STOP)).GetCurSel();
    cfg_onexit_power = ((CComboBox)GetDlgItem(IDC_CB_POWER_EXIT)).GetCurSel();

    uGetDlgItemText(*this, IDC_TX_PLAY_FORMAT1, cfg_onplay_format1);
    uGetDlgItemText(*this, IDC_TX_PLAY_FORMAT2, cfg_onplay_format2);
    uGetDlgItemText(*this, IDC_TX_LOAD_STRING1, cfg_onload_string1);
    uGetDlgItemText(*this, IDC_TX_LOAD_STRING2, cfg_onload_string2);
    uGetDlgItemText(*this, IDC_TX_STOP_STRING1, cfg_onstop_string1);
    uGetDlgItemText(*this, IDC_TX_STOP_STRING2, cfg_onstop_string2);

    cfg_spectrum_enable_onplay = ((CCheckBox)GetDlgItem(IDC_CK_SPEN_ONPLAY)).GetCheck();
    cfg_spectrum_enable_onstop = ((CCheckBox)GetDlgItem(IDC_CK_SPEN_ONSTOP)).GetCheck();
    cfg_spectrum_fft_speed = GetDlgItemInt(IDC_TX_SPFFTSPEED_STRING, NULL, false);
    cfg_spectrum_draw_speed = GetDlgItemInt(IDC_TX_SPDRAWSPEED_STRING, NULL, false);
    cfg_spectrum_pos_x = GetDlgItemInt(IDC_TX_SPPOSX_STRING, NULL, false);
    cfg_spectrum_len_x = GetDlgItemInt(IDC_TX_SPLENX_STRING, NULL, false);

    uGetDlgItemText(*this, IDC_TX_VOLUME_STRING2, cfg_onvolume_string2);
    cfg_onvolume_hold = GetDlgItemInt(IDC_TX_VOLUME_HOLD_STRING, NULL, false);

    cfg_fadein_speed = GetDlgItemInt(IDC_TX_FADEIN_STRING, NULL, false);
    cfg_scroll_wait = GetDlgItemInt(IDC_TX_SCROLL_WAIT_STRING, NULL, false);
    cfg_scroll_speed = GetDlgItemInt(IDC_TX_SCROLL_STRING, NULL, false);

    value_change = false;

    OnChanged(); //our dialog content has not changed but the flags have - our currently shown values now match the settings so the apply button can be disabled
}

bool CPreferences::HasChanged()
{
    return value_change;
}

void CPreferences::OnChanged()
{
    //tell the host that our state has changed to enable/disable the apply button appropriately.
    m_callback->on_state_changed();
}

void CPreferences::reset_from(UINT, int, CWindow)
{
    reset_FROM = true;
}

void CPreferences::write_from(UINT, int, CWindow)
{
    write_FROM = true;
}

class preferences_page_myimpl : public preferences_page_impl<CPreferences>
{
    // preferences_page_impl<> helper deals with instantiation of our dialog; inherits from preferences_page_v3.
public:
    const char* get_name()
    {
        return page_name;
    }
    GUID get_guid()
    {
        return guid_page;
    }
    GUID get_parent_guid()
    {
        return guid_display;
    }
};
static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_myimpl_factory;
