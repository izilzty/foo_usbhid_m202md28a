#include "stdafx.h"
#include "hidapi.h"
#include "display_thread.h"
#include "preferences.h"

DisplayThread dph;

class on_initquit : public initquit
{
public:
	void on_init()
	{
		hid_device* hid;
		wchar_t string_buffer[64];

		console::formatter() << "HID VFD - HIDAPI version: " << hid_version_str();
		hid = hid_open(cfg_usbhid_vid, cfg_usbhid_pid, NULL);
		if (hid != NULL)
		{
			console::info("HID VFD - HID device found");
			if (hid_get_manufacturer_string(hid, string_buffer, sizeof(string_buffer)) == 0)
			{
				console::formatter() << "HID VFD - HID device manufacturer: " << string_buffer;
			}
			if (hid_get_product_string(hid, string_buffer, sizeof(string_buffer)) == 0)
			{
				console::formatter() << "HID VFD - HID device product: " << string_buffer;
			}
			hid_close(hid);
			if (dph.start(cfg_usbhid_vid, cfg_usbhid_pid) == 0)
			{
				console::info("HID VFD - Thread started");
			}
			else
			{
				console::info("HID VFD - Thread fail");
			}
		}
		else
		{
			console::info("HID VFD - No HID device found");
		}
	}
	void on_quit()
	{
		dph.stop();
	}
};

static initquit_factory_t<on_initquit> g_initquit_factory;
