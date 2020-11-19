#include "m202md28a.h"
#include <stdarg.h> /* va_start() va_end() va_arg() */
#include <Windows.h> /* MultiByteToWideChar() WideCharToMultiByte() */

/**
 * @brief  打开HID设备。
 * @param  vid Vendor ID。
 * @param  pid Product ID。
 * @return hid_device* HID设备，打开失败时返回NULL。
 */
hid_device* display_init(uint16_t vid, uint16_t pid)
{
	hid_device* device;

	device = hid_open(vid, pid, NULL);

	return device;
}

/**
 * @brief  关闭HID设备。
 * @param  device HID设备。
 */
void display_deinit(hid_device* device)
{
	if (device == NULL)
	{
		return;
	}
	hid_close(device);
	device = NULL;
}

/**
 * @brief  向屏幕发送数据。
 * @param  device HID设备。
 * @param  data 要发送的数据指针。
 * @param  data_size 要发送的数据大小。
 * @return 0=成功，-1=失败。
 */
int8_t display_send_data(hid_device* device, const uint8_t* data, uint16_t data_size)
{
	uint8_t usb_send_data[1 + 64]; /* 一位HID Report ID + HID单次发送的数据 */
	uint8_t screen_max_data_size;
	uint16_t loop_count;
	uint16_t i;

	if (device == NULL)
	{
		return -1;
	}

	screen_max_data_size = sizeof(usb_send_data) - 2; /* 屏幕一次可接受的纯数据大小 */
	loop_count = data_size / screen_max_data_size; /* 发送全部数据所需循环次数 */

	for (i = 0; i < loop_count; i++)
	{
		memset(usb_send_data, '\0', sizeof(usb_send_data));
		usb_send_data[0] = DISPLAY_HID_REPORT_ID;
		usb_send_data[1] = screen_max_data_size;
		memcpy(usb_send_data + 2, data, screen_max_data_size);
		data += screen_max_data_size;
		data_size -= screen_max_data_size;
		if (hid_write(device, usb_send_data, sizeof(usb_send_data)) < sizeof(usb_send_data))
		{
			return -1;
		}
	}

	if (data_size != 0)
	{
		memset(usb_send_data, '\0', sizeof(usb_send_data));
		usb_send_data[0] = DISPLAY_HID_REPORT_ID;
		usb_send_data[1] = (uint8_t)data_size;
		memcpy(usb_send_data + 2, data, data_size);
		if (hid_write(device, usb_send_data, sizeof(usb_send_data)) < sizeof(usb_send_data))
		{
			return -1;
		}
	}

	return 0;
}

/**
 * @brief  向屏幕发送命令和参数。
 * @param  device HID设备。
 * @param  cmd 要发送的命令指针。
 * @param  cmd_size 要发送的命令大小。
 * @param  param_size 要发送的参数个数。
 * @param  ... 要发送的参数。
 * @return 0=成功，-1=失败。
 */
int8_t display_send_cmd(hid_device* device, const uint8_t* cmd, uint8_t cmd_size, uint8_t param_size, ...)
{
	uint8_t	cmd_data[63];
	uint8_t cmd_data_size;
	uint16_t i;
	va_list valist;

	if ((device == NULL) || ((cmd_size + param_size) > sizeof(cmd_data)))
	{
		return -1;
	}

	memset(cmd_data, '\0', sizeof(cmd_data));
	cmd_data_size = 0;

	memcpy(cmd_data + cmd_data_size, cmd, cmd_size);
	cmd_data_size += cmd_size;

	va_start(valist, param_size);
	for (i = 0; i < param_size; i++)
	{
		cmd_data[cmd_data_size] = (uint8_t)va_arg(valist, int);
		cmd_data_size += 1;
	}
	va_end(valist);

	if (display_send_data(device, cmd_data, cmd_data_size) != 0)
	{
		return -1;
	}

	return 0;
}

uint8_t display_read_state(hid_device* device)
{
	uint8_t usb_send_data[8];

	memset(usb_send_data, 0xFF, sizeof(usb_send_data));
	hid_read_timeout(device, usb_send_data, sizeof(usb_send_data), 5000);

	return usb_send_data[4];
}

/**
 * @brief  加载屏幕默认设置。
 * @param  device HID设备。
 * @return 0=成功，-1=失败。
 */
int8_t display_load_default_setting(hid_device* device)
{
	int8_t ret;

	if (device == NULL)
	{
		return -1;
	}

	ret = 0;
	ret += display_send_cmd(device, cmd_DisplayClear, sizeof(cmd_DisplayClear), 0); /* 清空显示 */
	ret += display_send_cmd(device, cmd_ReverseDisplay_P, sizeof(cmd_ReverseDisplay_P), 1, param_ReverseDisplay_Disable); /* 关闭颜色反显 */
	ret += display_send_cmd(device, cmd_CursorDisplay_P, sizeof(cmd_CursorDisplay_P), 1, param_CursorDisplay_Disable); /* 关闭光标显示 */
	ret += display_send_cmd(device, cmd_OverWriteMode, sizeof(cmd_OverWriteMode), 0); /* 使用覆写模式 */
	ret += display_send_cmd(device, cmd_FROMUserFont_P, sizeof(cmd_FROMUserFont_P), 1, param_FROMUserFont_Disable); /* 不使用用户定义字体 */
	ret += display_send_cmd(device, cmd_InternationalFontSet_P, sizeof(cmd_InternationalFontSet_P), 1, 0x00); /* 国际字符设为默认 */
	ret += display_send_cmd(device, cmd_CharacterTableType_P, sizeof(cmd_CharacterTableType_P), 1, 0x00); /* 单字节字符表设为默认 */
	ret += display_send_cmd(device, cmd_2ByteCharacter_P, sizeof(cmd_2ByteCharacter_P), 1, param_2ByteCharacter_Disable); /* 不使用双字节 */
	ret += display_send_cmd(device, cmd_BrightnessLevelSetting_P, sizeof(cmd_BrightnessLevelSetting_P), 1, param_BrightnessLevel_100); /* 亮度100% */
	ret += display_send_cmd(device, cmd_DisplayPower_P, sizeof(cmd_DisplayPower_P), 1, param_DisplayPower_Enable); /* 显示电源打开状态 */
	if (ret != 0)
	{
		return -1;
	}

	return ret;
}

/**
 * @brief  清空已写入的用户字体和已保存的设置。
 * @param  device HID设备。
 * @return 0=成功，-1=失败。
 */
int8_t display_reset_FROM(hid_device* device)
{
	display_send_cmd(device, cmd_InitializeDisplay, sizeof(cmd_InitializeDisplay), 0);
	display_load_default_setting(device);

	display_send_cmd(device, cmd_UserSetUpModeStart, sizeof(cmd_UserSetUpModeStart), 0);
	if (display_read_state(device) != 0x00)
	{
		return -1;
	}

	display_send_cmd(device, cmd_UserFontDeletAll, sizeof(cmd_UserFontDeletAll), 0);
	if (display_read_state(device) != 0x00)
	{
		return -1;
	}

	display_send_cmd(device, cmd_UserTableDeletAll, sizeof(cmd_UserTableDeletAll), 0);
	if (display_read_state(device) != 0x00)
	{
		return -1;
	}

	display_send_cmd(device, cmd_UserSetUpModeEnd, sizeof(cmd_UserSetUpModeEnd), 0);

	return 0;
}

/**
 * @brief  写入用户字体。
 * @param  device HID设备。
 * @return 0=成功，-1=失败。
 */
int8_t display_write_FROM(hid_device* device)
{
	uint8_t from_data[3846];
	uint8_t* from_data_ptr;

	display_send_cmd(device, cmd_InitializeDisplay, sizeof(cmd_InitializeDisplay), 0);
	display_load_default_setting(device);

	display_send_cmd(device, cmd_UserSetUpModeStart, sizeof(cmd_UserSetUpModeStart), 0);
	if (display_read_state(device) != 0x00)
	{
		return -1;
	}

	display_send_cmd(device, cmd_UserFontDeletAll, sizeof(cmd_UserFontDeletAll), 0);
	if (display_read_state(device) != 0x00)
	{
		return -1;
	}

	display_send_cmd(device, cmd_UserTableDeletAll, sizeof(cmd_UserTableDeletAll), 0);
	if (display_read_state(device) != 0x00)
	{
		return -1;
	}

	from_data_ptr = from_data;
	memset(from_data_ptr, 0xFF, sizeof(from_data));
	memcpy(from_data_ptr, cmd_UserTableRegistAll, sizeof(cmd_UserTableRegistAll));
	from_data_ptr += sizeof(cmd_UserTableRegistAll);
	memcpy(from_data_ptr, FROM_icon_stop, sizeof(FROM_icon_stop)); /* 0x80 */
	from_data_ptr += sizeof(FROM_icon_stop);
	memcpy(from_data_ptr, FROM_icon_play, sizeof(FROM_icon_play)); /* 0x81 */
	from_data_ptr += sizeof(FROM_icon_play);
	memcpy(from_data_ptr, FROM_icon_pause, sizeof(FROM_icon_pause)); /* 0x82 */
	from_data_ptr += sizeof(FROM_icon_pause);
	memcpy(from_data_ptr, FROM_icon_load, sizeof(FROM_icon_load)); /* 0x83 */
	from_data_ptr += sizeof(FROM_icon_load);

	from_data_ptr += 12 * (15 * 16 / 8);
	memcpy(from_data_ptr, FROM_icon_bar, sizeof(FROM_icon_bar)); /* 0x90 ~ 0x9F */
	from_data_ptr += sizeof(FROM_icon_bar);

	display_send_data(device, from_data, sizeof(from_data));
	if (display_read_state(device) != 0x00)
	{
		return -1;
	}

	display_send_cmd(device, cmd_UserSetUpModeEnd, sizeof(cmd_UserSetUpModeEnd), 0);

	return 0;
}

int8_t display_set_dim(hid_device* device, uint8_t level)
{
	if (device == NULL)
	{
		return -1;
	}

	if (level > 3)
	{
		level = 3;
	}

	if (display_send_cmd(device, cmd_BrightnessLevelSetting_P, sizeof(cmd_BrightnessLevelSetting_P), 1, level + 1) != 0)
	{
		return -1;
	}
	return 0;
}

int8_t display_set_power(hid_device* device, uint8_t power)
{
	if (device == NULL)
	{
		return -1;
	}

	if (power > 1)
	{
		power = 1;
	}

	if (display_send_cmd(device, cmd_DisplayPower_P, sizeof(cmd_DisplayPower_P), 1, power) != 0)
	{
		return -1;
	}
	return 0;
}

/**
 * @brief  清空屏幕显示。
 * @param  device HID设备。
 * @return 0=成功，-1=失败。
 */
int8_t display_clear_screen(hid_device* device)
{
	if (device == NULL)
	{
		return -1;
	}

	if (display_send_cmd(device, cmd_DisplayClear, sizeof(cmd_DisplayClear), 0) != 0)
	{
		return -1;
	}

	return 0;
}

/**
 * @brief  清空单行显示。
 * @param  device HID设备。
 * @return 0=成功，-1=失败。
 */
int8_t display_clear_line(hid_device* device, uint8_t line)
{
	int8_t ret;

	if (device == NULL)
	{
		return -1;
	}

	ret = 0;
	ret += display_send_cmd(device, cmd_CursorSet_2P, sizeof(cmd_CursorSet_2P), 2, 1, line + 1);
	ret += display_send_cmd(device, cmd_LineClear, sizeof(cmd_LineClear), 0);
	if (ret != 0)
	{
		return -1;
	}

	return 0;
}

/**
 * @brief  设置代码页为双字节。
 * @param  device HID设备。
 * @return 0=成功，-1=失败。
 */
int8_t display_set_cp_utf8(hid_device* device)
{
	int8_t ret;

	if (device == NULL)
	{
		return -1;
	}

	ret = 0;
	ret += display_send_cmd(device, cmd_2ByteCharacter_P, sizeof(cmd_2ByteCharacter_P), 1, param_2ByteCharacter_Enable);
	ret += display_send_cmd(device, cmd_CharacterTableType_P, sizeof(cmd_CharacterTableType_P), 1, 0x00);
	if (ret != 0)
	{
		return -1;
	}

	return 0;
}

/**
 * @brief  设置代码页为单字节ASCII。
 * @param  device HID设备。
 * @return 0=成功，-1=失败。
 */
int8_t display_set_cp_ascii(hid_device* device)
{
	int8_t ret;

	if (device == NULL)
	{
		return -1;
	}

	ret = 0;
	ret += display_send_cmd(device, cmd_2ByteCharacter_P, sizeof(cmd_2ByteCharacter_P), 1, param_2ByteCharacter_Disable);
	ret += display_send_cmd(device, cmd_CharacterTableType_P, sizeof(cmd_CharacterTableType_P), 1, 0x00);
	if (ret != 0)
	{
		return -1;
	}

	return 0;
}

/**
 * @brief  设置代码页为单字节用户字体。
 * @param  device HID设备。
 * @return 0=成功，-1=失败。
 */
int8_t display_set_cp_user(hid_device* device)
{
	int8_t ret;

	if (device == NULL)
	{
		return -1;
	}

	ret = 0;
	ret += display_send_cmd(device, cmd_2ByteCharacter_P, sizeof(cmd_2ByteCharacter_P), 1, param_2ByteCharacter_Disable);
	ret += display_send_cmd(device, cmd_CharacterTableType_P, sizeof(cmd_CharacterTableType_P), 1, 0xFF);
	if (ret != 0)
	{
		return -1;
	}

	return 0;
}

static int8_t search_codepage(const uint16_t* codepage, uint32_t codepage_byte_size, uint8_t codepage_id, uint16_t unicode, uint16_t* oem_code)
{
	uint32_t i;

	codepage_byte_size /= sizeof(uint16_t);
	for (i = 0; i < codepage_byte_size; i += 2)
	{
		if (codepage[i] == unicode)
		{
			oem_code[0] = codepage_id;
			oem_code[1] = codepage[i + 1];
			return 0;
		}
	}
	return -1;
}

static void in_gb2312_range(uint16_t* oem_code)
{
	if (!((oem_code[1] >> 8) >= 0xA1 && (oem_code[1] >> 8) <= 0xFE && (oem_code[1] & 0x00FF) >= 0xA1 && (oem_code[1] & 0x00FF) <= 0xFE))
	{
		oem_code[0] = 0;
		oem_code[1] = 0x0000;
	}
}

/**
 * @brief  显示UTF8字符串。
 * @param  device HID设备。
 * @param  x 显示起始X位置，从0开始。
 * @param  y 显示起始Y位置，从0开始。
 * @param  str_start_pos 要显示的字符起始位置，从0开始。
 * @param  str_draw_count 要显示的字符长度，从1开始，如果大于40或小于0则使用默认值40，为0没有任何显示。
 * @param  fill 当要绘制的字符小于指定数量时，是否使用指定字符进行填充
 * @param  str 要显示的字符串指针，必须存在结束符\0。
 * @return 0=成功，-1=失败。
 */
int8_t display_draw_utf8(hid_device* device, uint8_t x, uint8_t y, uint8_t str_start_pos, uint8_t str_draw_count, char fill, const char* str)
{
	int32_t i;
	uint16_t* unicode_str;
	int32_t unicode_str_len;
	uint16_t oem_code[2]; /* [0]存储字符内码码表类型，[1]存储字符内码 */
	uint8_t display_data[63]; /* 一次最多处理的数据量，达到后发送数据到屏幕，可增大或减小 */
	uint8_t* display_data_ptr;
	uint16_t display_data_size;
	uint8_t current_codepage;
	uint8_t fill_str_len;

	if (device == NULL)
	{
		return -1;
	}

	if (*str == '\0') /* 不处理以结束符开头的字符串 */
	{
		return 0;
	}

	unicode_str_len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0); /* 获取全部UTF8字符数量，第四个参数为-1时结束符\0会被计算 */
	if (unicode_str_len < 0)
	{
		return -1;
	}

	memset(display_data, 0x00, sizeof(display_data));
	display_data_ptr = display_data;
	display_data_size = 0;

	memcpy(display_data_ptr, cmd_CursorSet_2P, sizeof(cmd_CursorSet_2P)); /* 复制设置光标位置指令 */
	display_data_ptr += sizeof(cmd_CursorSet_2P);
	display_data_size += sizeof(cmd_CursorSet_2P);
	*display_data_ptr = x + 1; /* 设置光标位置X数据，从1开始 */
	display_data_ptr += 1;
	display_data_size += 1;
	*display_data_ptr = y + 1; /* 设置光标位置Y数据，从1开始 */
	display_data_ptr += 1;
	display_data_size += 1;

	fill_str_len = 0;
	if (str_start_pos < 0)
	{
		str_start_pos = 0;
	}
	if (str_draw_count < 0 || str_draw_count > 40)
	{
		str_draw_count = 40;
	}
	if (fill != '\0' && unicode_str_len - 1 - str_start_pos < str_draw_count)
	{
		if (unicode_str_len - 1 - str_start_pos <= 0)
		{
			fill_str_len = str_draw_count;
			str_start_pos = 0;
		}
		else
		{
			fill_str_len = str_draw_count - (unicode_str_len - 1 - str_start_pos);
		}
	}
	unicode_str = (uint16_t*)malloc((unicode_str_len + fill_str_len) * sizeof(uint16_t)); /* 分配UNICODE字符串空间 */
	if (unicode_str == NULL)
	{
		return -1;
	}
	MultiByteToWideChar(CP_UTF8, 0, str, -1, unicode_str, unicode_str_len); /* UTF8转UNICODE字符串，第四个参数为-1时结束符\0会被复制，无需手动添加 */
	if (fill_str_len > 0)
	{
		for (i = 0; i < fill_str_len; i++)
		{
			unicode_str[unicode_str_len - 1 + i] = fill;
		}
		unicode_str[unicode_str_len - 1 + fill_str_len] = '\0';
		unicode_str_len += fill_str_len;
	}

	current_codepage = 0;
	for (i = 0; i < unicode_str_len - 1; i++) /* 因为屏幕的中日韩代码页是分开的，所以需要确认每个字符并切换到相应的代码页 */
	{
		if (i < str_start_pos || i >= (str_start_pos + str_draw_count)) /* 限制显示起始字符和最大显示长度 */
		{
			continue;
		}
		if (display_data_size > (sizeof(display_data) - sizeof(cmd_2ByteCharacterType_P) - 1 - 2) || display_data_size == 0) /* 待发送的数据超出单次允许填充的大小，发送一次数据。 */
		{
			if (display_data_size != 0)
			{
				if (display_send_data(device, display_data, display_data_size) != 0) /* 如果每个字符都切换代码页，那么显示一屏要发四次数据。 */
				{
					free(unicode_str);
					return -1;
				}
			}
			memset(display_data, 0x00, sizeof(display_data)); /* 初始化待发送数据 */
			display_data_ptr = display_data;
			display_data_size = 0;
		}
		oem_code[0] = 0;
		oem_code[1] = 0x0000;
		if (unicode_str[i] < 0x007F) /* ASCII字符 */
		{
			oem_code[0] = 5;
			oem_code[1] = unicode_str[i];
		}
		/* 需要注意，中文代码页里虽然有日文字符，但是字体样式和日语代码页内的不同，所以搜索顺序会在一定范影响显示效果，可以自行替换 SHIFTJIS日语 和 GB2312简体中文 的位置来查看差异 */
		if (oem_code[0] == 0) /* GB2312简体中文 */
		{
			search_codepage(uni2oem936, sizeof(uni2oem936), 3, unicode_str[i], oem_code);
			in_gb2312_range(oem_code); /* 936码表范围是GBK，限制到屏幕支持的GB2312范围 */
		}
		if (oem_code[0] == 0) /* BIG5繁体中文 */
		{
			search_codepage(uni2oem950, sizeof(uni2oem950), 4, unicode_str[i], oem_code);
		}
		if (oem_code[0] == 0) /* SHIFTJIS日语 */
		{
			search_codepage(uni2oem932, sizeof(uni2oem932), 1, unicode_str[i], oem_code);
		}
		if (oem_code[0] == 0) /* KSC5601韩语 */
		{
			search_codepage(uni2oem949, sizeof(uni2oem949), 2, unicode_str[i], oem_code);
		}
		if (oem_code[0] != 0)
		{
			if ((oem_code[0] != current_codepage) && (oem_code[0] != 5)) /* 复制代码页切换命令，不为ASCII专门更改，使用最后所设置的值。 */
			{
				memcpy(display_data_ptr, cmd_2ByteCharacterType_P, sizeof(cmd_2ByteCharacterType_P));
				display_data_ptr += sizeof(cmd_2ByteCharacterType_P);
				*display_data_ptr = oem_code[0] - 1; /* 程序判断所使用的代码页范围为1 ~ 4（5表示ASCII，无操作），屏幕使用的代码页范围为0 ~ 3，进行匹配。 */
				display_data_ptr += 1;
				display_data_size += sizeof(cmd_2ByteCharacterType_P) + 1;
			}
			current_codepage = oem_code[0] & 0x00FF;
			switch (oem_code[0]) /* 复制要显示的内容 */
			{
			case 1: /* 双字节 */
			case 2:
			case 3:
			case 4:
				display_data_size += 2;
				*display_data_ptr = oem_code[1] >> 8;
				display_data_ptr += 1;
				*display_data_ptr = oem_code[1] & 0x00FF;
				display_data_ptr += 1;
				break;
			case 5: /* ASCII */
				if (oem_code[1] != 0x0000)
				{
					display_data_size += 1;
					*display_data_ptr = oem_code[1] & 0x00FF;
					display_data_ptr += 1;
				}
				break;
			}
		}
		else /* 不支持的编码，如特殊符号等，使用特定ASCII字符填充 */
		{
			display_data_size += 1;
			*display_data_ptr = '?';
			display_data_ptr += 1;
		}
	}
	free(unicode_str);
	if (display_data_size != 0) /* 最后一次发送剩余的数据。 */
	{
		if (display_send_data(device, display_data, display_data_size) != 0)
		{
			return -1;
		}
	}

	return 0;
}

/**
 * @brief  显示ASCII字符串，可用于显示用户字体。
 * @param  device HID设备。
 * @param  x 显示起始X位置，从0开始。
 * @param  y 显示起始Y位置，从0开始。
 * @param  str_start_pos 要显示的字符起始位置，从0开始。
 * @param  str_draw_count 要显示的字符长度，从1开始，如果大于40或小于0则使用默认值40，为0没有任何显示。
 * @param  fill 当要绘制的字符小于指定数量时，是否使用指定字符进行填充
 * @param  str 要显示的字符串指针，必须存在结束符\0。
 * @return 0=成功，-1=失败。
 */
int8_t display_draw_ascii(hid_device* device, uint8_t x, uint8_t y, uint8_t str_start_pos, uint8_t str_draw_count, char fill, const char* str)
{
	int32_t i;
	int32_t str_len;
	uint8_t display_data[44]; /* 一次处理的数据量，大于等于一屏要发送的数据即可 */
	uint8_t* display_data_ptr;
	uint16_t display_data_size;
	uint8_t fill_str_len;

	if (device == NULL)
	{
		return -1;
	}

	if (*str == '\0') /* 不处理以结束符开头的字符串 */
	{
		return 0;
	}

	str_len = strlen(str) + 1; /* 获取全部字符数量，不包括结束符\0，手动加1 */
	if (str_len < 0)
	{
		return -1;
	}

	memset(display_data, 0x00, sizeof(display_data));
	display_data_ptr = display_data;
	display_data_size = 0;

	memcpy(display_data_ptr, cmd_CursorSet_2P, sizeof(cmd_CursorSet_2P)); /* 填充设置光标位置指令 */
	display_data_ptr += sizeof(cmd_CursorSet_2P);
	display_data_size += sizeof(cmd_CursorSet_2P);

	*display_data_ptr = x + 1; /* 设置光标位置X数据，从1开始 */
	display_data_ptr += 1;
	display_data_size += 1;

	*display_data_ptr = y + 1; /* 设置光标位置Y数据，从1开始 */
	display_data_ptr += 1;
	display_data_size += 1;

	fill_str_len = 0;
	if (str_start_pos < 0)
	{
		str_start_pos = 0;
	}
	if (str_draw_count < 0 || str_draw_count > 40)
	{
		str_draw_count = 40;
	}
	if (fill != '\0' && str_len - 1 - str_start_pos < str_draw_count)
	{
		if (str_len - 1 - str_start_pos <= 0)
		{
			fill_str_len = str_draw_count;
			str_start_pos = 0;
		}
		else
		{
			fill_str_len = str_draw_count - (str_len - 1 - str_start_pos);
		}
	}

	for (i = 0; i < str_len + fill_str_len - 1; i++) /* 填充字符 */
	{
		if (i < str_start_pos || i >= (str_start_pos + str_draw_count)) /* 限制显示起始字符和最大显示长度 */
		{
			continue;
		}
		if (display_data_size < sizeof(display_data))
		{
			display_data_size += 1;
			if (i < str_len - 1)
			{
				*display_data_ptr = str[i];

			}
			else
			{
				*display_data_ptr = fill;
			}
			display_data_ptr += 1;
		}
		else
		{
			break;
		}
	}
	if (display_data_size != 0) /* 发送显示数据。 */
	{
		if (display_send_data(device, display_data, display_data_size) != 0)
		{
			return -1;
		}
	}

	return 0;
}

void scroll_unicode_left(uint16_t* unicode_src, uint32_t unicode_src_len, uint16_t* unicode_dst, uint32_t unicode_dst_len, uint32_t scroll_len)
{
	uint32_t i;
	uint32_t unicode_str_pos;

	unicode_src_len -= 1;
	unicode_dst_len -= 1;

	unicode_str_pos = scroll_len % unicode_src_len;

	for (i = 0; i < unicode_dst_len; i++)
	{
		unicode_dst[i] = *(unicode_src + unicode_str_pos);
		unicode_str_pos += 1;
		if (unicode_str_pos >= unicode_src_len)
		{
			unicode_str_pos = 0;
		}
	}
	unicode_dst[unicode_dst_len] = '\0';
}

void scroll_unicode_right(uint16_t* unicode_src, uint32_t unicode_src_len, uint16_t* unicode_dst, uint32_t unicode_dst_len, uint32_t scroll_len)
{
	uint32_t i;

	unicode_src_len -= 1;
	unicode_dst_len -= 1;

	for (i = 0; i < unicode_dst_len; i++)
	{
		if (i - scroll_len <= unicode_src_len && i >= scroll_len)
		{
			unicode_dst[i] = *unicode_src;
			unicode_src += 1;
		}
		else
		{
			unicode_dst[i] = ' ';
		}
	}
	unicode_dst[unicode_dst_len] = '\0';
}

void display_scroll_utf8(const char* src_str, int16_t move_len, uint8_t display_line_len, char* dist_str, uint16_t dist_str_max_size)
{
	uint16_t* unicode_str;
	uint16_t* unicode_str_dist;
	int32_t unicode_str_len;

	unicode_str_len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0); /* 获取全部UTF8字符数量，第四个参数为-1时结束符\0会被计算 */
	if (unicode_str_len < 0)
	{
		return;
	}
	unicode_str = (uint16_t*)malloc(unicode_str_len * sizeof(uint16_t)); /* 分配UNICODE字符串空间 */
	unicode_str_dist = (uint16_t*)malloc(display_line_len * sizeof(uint16_t)); /* 分配UNICODE字符串空间 */
	if (unicode_str == NULL || unicode_str_dist == NULL)
	{
		return;
	}
	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, unicode_str, unicode_str_len); /* UTF8转UNICODE字符串，第四个参数为-1时结束符\0会被复制，无需手动添加 */

	if (move_len < 0)
	{
		scroll_unicode_left(unicode_str, unicode_str_len, unicode_str_dist, display_line_len, -move_len);
	}
	else
	{
		scroll_unicode_right(unicode_str, unicode_str_len, unicode_str_dist, display_line_len, move_len);
	}

	WideCharToMultiByte(CP_UTF8, 0, unicode_str_dist, -1, dist_str, dist_str_max_size, NULL, NULL);
	//dist_str[dist_str_max_size - 1] = '\0';

	free(unicode_str);
	free(unicode_str_dist);
}

uint32_t display_get_utf8_len(const char* src_str, uint8_t with_space)
{
	int32_t i;
	uint16_t* unicode_str;
	int32_t unicode_str_len;

	unicode_str_len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0); /* 获取全部UTF8字符数量，第四个参数为-1时结束符\0会被计算 */
	if (unicode_str_len < 0)
	{
		return 0;
	}
	unicode_str = (uint16_t*)malloc(unicode_str_len * sizeof(uint16_t)); /* 分配UNICODE字符串空间 */
	if (unicode_str == NULL)
	{
		return 0;
	}
	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, unicode_str, unicode_str_len); /* UTF8转UNICODE字符串，第四个参数为-1时结束符\0会被复制，无需手动添加 */

	for (i = unicode_str_len - 1; i > 0; i--)
	{
		if (with_space == 0)
		{
			if (unicode_str[i] == ' ' || unicode_str[i] == '\0')
			{
				unicode_str_len -= 1;
			}
			else
			{
				break;
			}
		}
		else
		{
			unicode_str_len -= 1;
			break;
		}
	}

	free(unicode_str);

	return (uint32_t)unicode_str_len;
}
