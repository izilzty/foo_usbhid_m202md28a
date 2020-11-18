#include "display_thread.h"
#include "m202md28a.h"
#include "preferences.h"
#include <stdio.h>
#include <time.h>

#define LINE1_START_X 2
#define LINE1_LEN 18

#define LINE2_START_X 0
#define LINE2_LEN 20

#define STATEICON_X 0
#define STATEICON_Y 0
#define STATEICON_FILL 2
#define STATEICON_STOP "\x80"
#define STATEICON_PLAY "\x81"
#define STATEICON_PAUSE "\x82"
#define STATEICON_LOAD "\x83"

#define VOLUME_START_X 0
#define VOLUME_START_Y 1
#define VOLUME_LEN 20
#define VOLUME_LINE_SIZE VOLUME_LEN

#define SCROLL_DEFAULT_SPEED 300
#define SCROLL_LINE1_SIZE LINE1_LEN

DWORD WINAPI main_display_thread(LPVOID lpParamter)
{
	hid_device* hid;
	DisplayThreadData* thread_data;
	PlayInfo* play_info;
	clock_t start_clock, end_clock;
	uint16_t code_exec_time;
	bool disable_display;

	uint32_t i;

	char scroll_str[(SCROLL_LINE1_SIZE + 1) * 3];
	uint16_t scroll_delay_count;
	uint16_t scroll_delay_move;
	int32_t scroll_pos;

	char volume_str[(VOLUME_LINE_SIZE + 1) * 3];
	int32_t volume_delay_count;

	thread_data = (DisplayThreadData*)lpParamter;
	play_info = &thread_data->play_info;
	hid = display_init(thread_data->vid, thread_data->pid);
	if (hid != NULL)
	{
		thread_data->status = 1;
	}
	else
	{
		thread_data->status = -1;
		return -1;
	}

	display_load_default_setting(hid);
	display_set_cp_utf8(hid);

	scroll_delay_count = 0;
	scroll_delay_move = 0;
	scroll_pos = SCROLL_LINE1_SIZE - 1;
	code_exec_time = 0;

	play_info->update_play_state = true; /* 强制更新一次显示 */
	play_info->play_state = PLAY_STATE_STOP;
	disable_display = false;
	memset(scroll_str, '\0', sizeof(scroll_str));
	volume_delay_count = 0;

	while (thread_data->stop == 0)
	{
		start_clock = clock();

		if (play_info->is_new_track == true)
		{
			if (cfg_fadein_speed != 0)
			{
				scroll_pos = SCROLL_LINE1_SIZE - 1;
			}
			else
			{
				scroll_pos = 0;
			}
			scroll_delay_count = cfg_fadein_speed;
			scroll_delay_move = cfg_fadein_speed;
			play_info->is_new_track = false;
		}

		if (scroll_delay_count + code_exec_time < scroll_delay_move)
		{
			scroll_delay_count += code_exec_time;
		}
		else
		{
			if (play_info->str_line1_avaliable == true && (play_info->play_state == PLAY_STATE_PLAY || play_info->play_state == PLAY_STATE_PAUSE))
			{
				scroll_delay_count = 0;
				if (scroll_pos > 0)
				{
					scroll_delay_move = cfg_fadein_speed;
				}
				else if (scroll_pos == 0)
				{
					scroll_delay_move = cfg_scroll_wait;
				}
				else
				{
					if (display_get_utf8_len(play_info->str_line_1, 0) > SCROLL_LINE1_SIZE)
					{
						if (cfg_scroll_speed == 0 && (-scroll_pos % (display_get_utf8_len(play_info->str_line_1, 1) + 1)) == 0)
						{
							scroll_delay_move = -1;
							continue;
						}
						else if (cfg_scroll_speed != 0)
						{
							scroll_delay_move = cfg_scroll_speed;
						}
						else
						{
							scroll_delay_move = SCROLL_DEFAULT_SPEED;
						}
					}
					else
					{
						scroll_delay_move = -1;
						continue;
					}
				}
				display_scroll_utf8(play_info->str_line_1, scroll_pos, sizeof(scroll_str) / 3, scroll_str, sizeof(scroll_str));
				if (disable_display == false)
				{
					display_draw_utf8(hid, LINE1_START_X, 0, 0, LINE1_LEN, ' ', scroll_str);
				}
				scroll_pos -= 1;
			}
		}

		if (play_info->update_str_line2 == true)
		{
			if (disable_display == false && volume_delay_count <= 0)
			{
				display_draw_utf8(hid, LINE2_START_X, 1, 0, LINE2_LEN, ' ', play_info->str_line_2);
				play_info->update_str_line2 = false;
			}
		}

		if (play_info->update_volume == true)
		{
			if (cfg_onvolume_hold > 0)
			{
				volume_delay_count = cfg_onvolume_hold;
				memset(volume_str, '\0', sizeof(volume_str));
				for (i = 0; i < strlen(cfg_onvolume_string2); i++)
				{
					if (i < sizeof(volume_str) - 1)
					{
						volume_str[i] = cfg_onvolume_string2[i];
					}
					else
					{
						break;
					}
				}
				if (play_info->volume <= -100.0)
				{
					play_info->volume = -99.99;
				}
				snprintf(&volume_str[i], sizeof(volume_str) - i, "%+06.2fdB", (double)play_info->volume);
				display_draw_utf8(hid, VOLUME_START_X, VOLUME_START_Y, 0, VOLUME_LINE_SIZE, ' ', volume_str);
			}
			play_info->update_volume = false;
		}
		if (volume_delay_count - code_exec_time > 0)
		{
			volume_delay_count -= code_exec_time;
		}
		else if (volume_delay_count >= 0)
		{
			volume_delay_count = -1;
			if (play_info->play_state != PLAY_STATE_STOP)
			{
				play_info->update_str_line2 = true;
			}
			else
			{
				play_info->update_play_state = true;
			}
		}

		if (play_info->update_play_state == true)
		{
			switch (play_info->play_state)
			{
			case PLAY_STATE_STOP:
				disable_display = false;
				if (cfg_onstop_power <= 3)
				{
					display_set_power(hid, 1);
					display_set_dim(hid, cfg_onstop_power);
				}
				else if (cfg_onstop_power == 4)
				{
					display_set_power(hid, 1);
					display_clear_screen(hid);
					disable_display = true;
					break;
				}
				else if (cfg_onstop_power == 5)
				{
					display_set_power(hid, 0);
				}
				display_set_cp_user(hid);
				display_draw_ascii(hid, STATEICON_X, STATEICON_Y, 0, STATEICON_FILL, ' ', STATEICON_STOP);
				display_set_cp_utf8(hid);
				if (*cfg_onstop_string1 != '\0')
				{
					display_draw_utf8(hid, LINE1_START_X, 0, 0, LINE1_LEN, ' ', cfg_onstop_string1);
				}
				else
				{
					display_draw_utf8(hid, LINE1_START_X, 0, 0, LINE1_LEN, ' ', scroll_str);
				}
				if (*cfg_onstop_string2 != '\0')
				{
					display_draw_utf8(hid, LINE2_START_X, 1, 0, LINE2_LEN, ' ', cfg_onstop_string2);
				}
				else
				{
					if (play_info->str_line2_avaliable == true)
					{
						display_draw_utf8(hid, LINE2_START_X, 1, 0, LINE2_LEN, ' ', play_info->str_line_2);
					}
				}
				memset(scroll_str, '\0', sizeof(scroll_str));
				break;
			case PLAY_STATE_PLAY:
				if (cfg_onplay_power <= 3)
				{
					display_set_power(hid, 1);
					display_set_dim(hid, cfg_onplay_power);
					if (disable_display == true)
					{
						display_draw_utf8(hid, LINE1_START_X, 0, 0, LINE1_LEN, ' ', scroll_str);
						if (play_info->str_line2_avaliable == true)
						{
							display_draw_utf8(hid, LINE2_START_X, 1, 0, LINE2_LEN, ' ', play_info->str_line_2);
						}
						disable_display = false;
					}
				}
				else if (cfg_onplay_power == 4)
				{
					display_set_power(hid, 1);
					display_clear_screen(hid);
					disable_display = true;
					break;
				}
				else if (cfg_onplay_power == 5)
				{
					display_set_power(hid, 0);
				}
				display_set_cp_user(hid);
				display_draw_ascii(hid, STATEICON_X, STATEICON_Y, 0, STATEICON_FILL, ' ', STATEICON_PLAY);
				display_set_cp_utf8(hid);
				break;
			case PLAY_STATE_PAUSE:
				if (cfg_onpause_power <= 3)
				{
					display_set_power(hid, 1);
					display_set_dim(hid, cfg_onpause_power);
					if (disable_display == true)
					{
						display_draw_utf8(hid, LINE1_START_X, 0, 0, LINE1_LEN, ' ', scroll_str);
						if (play_info->str_line2_avaliable == true)
						{
							display_draw_utf8(hid, LINE2_START_X, 1, 0, LINE2_LEN, ' ', play_info->str_line_2);
						}
						disable_display = false;
					}
				}
				else if (cfg_onpause_power == 4)
				{
					display_set_power(hid, 1);
					display_clear_screen(hid);
					disable_display = true;
					break;
				}
				else if (cfg_onpause_power == 5)
				{
					display_set_power(hid, 0);
				}
				display_set_cp_user(hid);
				display_draw_ascii(hid, STATEICON_X, STATEICON_Y, 0, STATEICON_FILL, ' ', STATEICON_PAUSE);
				display_set_cp_utf8(hid);
				break;
			case PLAY_STATE_LOADING:
				if (disable_display == false)
				{
					display_set_cp_user(hid);
					display_draw_ascii(hid, STATEICON_X, STATEICON_Y, 0, STATEICON_FILL, ' ', STATEICON_LOAD);
					display_set_cp_utf8(hid);
					if (*cfg_onload_string1 != '\0')
					{
						display_draw_utf8(hid, LINE1_START_X, 0, 0, LINE1_LEN, ' ', cfg_onload_string1);
					}
					if (*cfg_onload_string2 != '\0')
					{
						display_draw_utf8(hid, LINE2_START_X, 1, 0, LINE2_LEN, ' ', cfg_onload_string2);
					}
				}
				break;
			}
			play_info->update_play_state = false;
		}

		if (write_FROM == true || reset_FROM == true)
		{
			while (thread_data->stop == 0)
			{
				if (write_FROM == true)
				{
					console::info("HID VFD - FROM write start");
					if (display_write_FROM(hid) != -1)
					{
						Sleep(3000);
						console::info("HID VFD - FROM write success");
						display_draw_ascii(hid, 0, 0, 0, 20, ' ', "FROM WRITE SUCCESS");
					}
					else
					{
						Sleep(3000);
						console::info("HID VFD - FROM write fail");
						display_draw_ascii(hid, 0, 0, 0, 20, ' ', "FROM WRITE FAIL");
					}
					display_draw_ascii(hid, 0, 1, 0, 20, ' ', "PLEASE RESTART FB2K");
					write_FROM = false;
					reset_FROM = false;
				}
				else if (reset_FROM == true)
				{
					console::info("HID VFD - FROM reset start");
					if (display_reset_FROM(hid) != -1)
					{
						Sleep(3000);
						console::info("HID VFD - FROM reset success");
						display_draw_ascii(hid, 0, 0, 0, 20, ' ', "FROM RESET SUCCESS");
					}
					else
					{
						Sleep(3000);
						console::info("HID VFD - FROM reset fail");
						display_draw_ascii(hid, 0, 0, 0, 20, ' ', "FROM RESET FAIL");
					}
					reset_FROM = false;
					write_FROM = false;
				}
				Sleep(100);
			}
		}

		Sleep(1);

		end_clock = clock();
		code_exec_time = (uint16_t)(end_clock - start_clock);
	}
	if (cfg_onexit_power <= 3)
	{
		display_set_power(hid, 1);
		display_set_dim(hid, cfg_onexit_power);
		display_draw_utf8(hid, LINE1_START_X, 0, 0, LINE1_LEN, ' ', cfg_onstop_string1);
		display_draw_utf8(hid, LINE2_START_X, 1, 0, LINE2_LEN, ' ', cfg_onstop_string2);
	}
	else if (cfg_onexit_power == 4)
	{
		display_set_power(hid, 1);
		display_clear_screen(hid);
	}
	else if (cfg_onexit_power == 5)
	{
		display_set_power(hid, 0);
	}
	display_deinit(hid);
	thread_data->stop = 0;
	console::info("HID VFD - Thread stop");
	return 0;
}

int DisplayThread::start(unsigned short vid, unsigned short pid)
{
	uint32_t timeout_counter;

	memset(&thread_data, 0x00, sizeof(DisplayThreadData));
	thread_data.vid = vid;
	thread_data.pid = pid;
	thread_handle = CreateThread(NULL, 0, main_display_thread, &thread_data, 0, NULL);
	if (thread_handle == NULL)
	{
		return -1;
	}
	timeout_counter = 0;
	while (true)
	{
		if (thread_data.status == 1)
		{
			OutputDebugStringA("Display thread running\n");
			static_api_ptr_t<play_callback_manager>()->register_callback(this, this->get_flags(), false);
			return 0;
		}
		else if (thread_data.status == -1)
		{
			OutputDebugStringA("Display thread start fail\n");
			return -1;
		}
		else
		{
			if (timeout_counter >= 3000)
			{
				OutputDebugStringA("Display thread start fail\n");
				return -1;
			}
			timeout_counter++;
			Sleep(1);
		}
	}
	return -1;
}

int DisplayThread::stop(void)
{
	uint32_t timeout_counter;

	if (thread_handle == NULL || thread_handle == INVALID_HANDLE_VALUE || thread_data.status < 0)
	{
		return 0;
	}
	static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
	thread_data.stop = 1;
	timeout_counter = 0;
	while (thread_data.stop == 1)
	{
		if (timeout_counter >= 3000)
		{
			OutputDebugStringA("Display thread stop fail\n");
			return -1;
		}
		timeout_counter++;
		Sleep(1);
	}
	CloseHandle(thread_handle);
	thread_handle = INVALID_HANDLE_VALUE;
	OutputDebugStringA("Display thread exit\n");
	return 0;
}

void DisplayThread::format_line1(const char* format)
{
	uint32_t formatter_str_len;
	titleformat_object::ptr formatter_script;
	pfc::string_formatter formatter_str;
	static_api_ptr_t<playback_control> m_playback_control;
	static_api_ptr_t<titleformat_compiler>()->compile_safe_ex(formatter_script, format);
	if (m_playback_control->playback_format_title(NULL, formatter_str, formatter_script, NULL, playback_control::display_level_all) == true)
	{
		thread_data.play_info.str_line1_avaliable = false;
		memset(thread_data.play_info.str_line_1, '\0', sizeof(thread_data.play_info.str_line_1));
		formatter_str_len = strlen(formatter_str);
		if (formatter_str_len >= sizeof(thread_data.play_info.str_line_1))
		{
			memcpy(thread_data.play_info.str_line_1, formatter_str, sizeof(thread_data.play_info.str_line_1) - 1);
		}
		else
		{
			memcpy(thread_data.play_info.str_line_1, formatter_str, formatter_str_len);
		}
		thread_data.play_info.str_line1_avaliable = true;
	}
}

void DisplayThread::format_line2(const char* format)
{
	uint32_t formatter_str_len;
	titleformat_object::ptr formatter_script;
	pfc::string_formatter formatter_str;
	static_api_ptr_t<playback_control> m_playback_control;
	static_api_ptr_t<titleformat_compiler>()->compile_safe_ex(formatter_script, format);
	if (m_playback_control->playback_format_title(NULL, formatter_str, formatter_script, NULL, playback_control::display_level_all) == true)
	{
		thread_data.play_info.str_line2_avaliable = false;
		memset(thread_data.play_info.str_line_2, '\0', sizeof(thread_data.play_info.str_line_2));
		formatter_str_len = strlen(formatter_str);
		if (formatter_str_len >= sizeof(thread_data.play_info.str_line_2))
		{
			memcpy(thread_data.play_info.str_line_2, formatter_str, sizeof(thread_data.play_info.str_line_2) - 1);
		}
		else
		{
			memcpy(thread_data.play_info.str_line_2, formatter_str, formatter_str_len);
		}
		thread_data.play_info.str_line2_avaliable = true;
		thread_data.play_info.update_str_line2 = true;
	}
}

void DisplayThread::on_playback_starting(play_control::t_track_command p_command, bool p_paused)
{
	format_line1(cfg_onplay_format1);
	format_line2(cfg_onplay_format2);
	thread_data.play_info.play_state = PLAY_STATE_LOADING;
	thread_data.play_info.update_play_state = true;
}

void DisplayThread::on_playback_new_track(metadb_handle_ptr p_track)
{
	static_api_ptr_t<playback_control> m_playback_control;
	format_line1(cfg_onplay_format1);
	format_line2(cfg_onplay_format2);
	thread_data.play_info.volume = m_playback_control->get_volume();
	thread_data.play_info.play_state = PLAY_STATE_PLAY;
	thread_data.play_info.is_new_track = true;
	thread_data.play_info.update_play_state = true;
}

void DisplayThread::on_playback_stop(play_control::t_stop_reason p_reason)
{
	if (p_reason != play_control::stop_reason_starting_another)
	{
		thread_data.play_info.play_state = PLAY_STATE_STOP;
		thread_data.play_info.update_play_state = true;
	}
}

void DisplayThread::on_playback_pause(bool p_state)
{
	if (p_state == false)
	{
		thread_data.play_info.play_state = PLAY_STATE_PLAY;
	}
	else
	{
		thread_data.play_info.play_state = PLAY_STATE_PAUSE;
	}
	thread_data.play_info.update_play_state = true;
}

void DisplayThread::on_playback_edited(metadb_handle_ptr p_track)
{
	thread_data.play_info.is_new_track = true;
	format_line1(cfg_onplay_format1);
}

void DisplayThread::on_playback_time(double p_time)
{
	format_line2(cfg_onplay_format2);
}

void DisplayThread::on_volume_change(float p_new_val)
{
	thread_data.play_info.volume = p_new_val;
	thread_data.play_info.update_volume = true;
}

unsigned int DisplayThread::get_flags(void)
{
	return ((flag_on_playback_all | flag_on_volume_change) & ~flag_on_playback_dynamic_info & ~flag_on_playback_dynamic_info_track & ~flag_on_playback_seek);
}









/* 以下为测试时使用的功能，仅供参考。 按Ctrl+K+C注译选中的代码，Ctrl+K+U取消注译选中的代码。 */

///* 直接从META或文件名获取当前轨道标题 */
//
//		const char* title_meta;
//		unsigned int title_meta_size;
//		metadb_handle_ptr metadb;
//		file_info_impl meta_info;
//
//		if (m_playback_control->get_now_playing(metadb) == true && metadb != NULL)
//		{
//			metadb->get_info(meta_info);
//			title_meta = meta_info.meta_get("TITLE", 0);
//			if (title_meta != NULL)
//			{
//				title_meta_size = strlen(title_meta);
//			}
//			else
//			{
//				title_meta = metadb->get_path();
//				title_meta = strrchr(title_meta, '\\') + 1;
//				title_meta_size = strchr(title_meta, '.') - title_meta;
//			}
//			strncpy_s(play_info.title, title_meta, sizeof(play_info.title));
//			play_info.title_avaliable = 1;
//			if (m_playback_control->is_paused() == true)
//			{
//				play_info.play_state = 2;
//			}
//			else
//			{
//				play_info.play_state = 1;
//			}
//		}


///* 直接获取获取正在播放的文件信息，不使用foobar格式化功能，仅在正在播放时有效 */
//metadb_handle_ptr metadb;
//file_info_impl meta_info;
//const char* codec;
//if (m_playback_control->get_now_playing(metadb) == true && metadb != NULL)
//{
//	if (metadb->is_info_loaded() == true)
//	{
//		metadb->get_info(meta_info);
//		play_info.bitrate = meta_info.info_get_int("bitrate");
//		play_info.samplerate = meta_info.info_get_int("samplerate");
//		codec = meta_info.info_get("codec");
//		if (strlen(codec) >= sizeof(play_info.str_codec))
//		{
//			memcpy(play_info.str_codec, codec, sizeof(play_info.str_codec) - 1);
//		}
//		else
//		{
//			memcpy(play_info.str_codec, codec, strlen(codec));
//		}
//	}
//	else
//	{
//		strncpy_s(play_info.str_codec, u8"----", sizeof(play_info.str_codec)); /* 文件解码失败 */
//	}

//旧测试显示
//if (playback_info_callback->play_info.volume != play_info.volume)
//{
//	play_info.volume = playback_info_callback->play_info.volume;
//	if (play_info.volume > -100.0)
//	{
//		snprintf(volume_str, sizeof(volume_str), "%06.2fdB", play_info.volume);
//	}
//	else
//	{
//		snprintf(volume_str, sizeof(volume_str), "%06.1fdB", play_info.volume);
//	}
//	//display_draw_utf8(hid, 0, 1, 0, 8, volume_str);
//	OutputDebugString(L"Update Volume\r\n");
//}
//if (playback_info_callback->play_info.play_state != play_info.play_state || playback_info_callback->play_info.playback_time != play_info.playback_time || playback_info_callback->play_info.track_length != play_info.track_length)
//{
//	play_info.play_state = playback_info_callback->play_info.play_state;
//	if (playback_info_callback->play_info.playback_time != play_info.playback_time || playback_info_callback->play_info.track_length != play_info.track_length)
//	{
//		play_info.playback_time = playback_info_callback->play_info.playback_time;
//		play_info.track_length = playback_info_callback->play_info.track_length;
//		time[0] = (unsigned char)(play_info.playback_time / 60.0);
//		time[1] = (unsigned char)(play_info.playback_time - time[0] * 60);
//		snprintf(time_str, sizeof(time_str), "%02d:%02d", time[0], time[1]);
//	}
//	if (play_info.play_state == 0)
//	{
//		snprintf(time_str, sizeof(time_str), "%02d:%02d", 0, 0);
//	}
//	if (play_info.play_state == 2)
//	{
//		display_set_cp_user(hid);
//		display_draw_utf8(hid, 0, 0, 0, 2, u8"\x81 ");
//		display_set_cp_utf8(hid);
//	}
//	else if (play_info.play_state == 1)
//	{
//		display_draw_utf8(hid, 0, 0, 0, 2, u8"▷ ");
//	}
//	display_draw_utf8(hid, 15, 1, 0, 5, time_str);
//	OutputDebugString(L"Update Time\r\n");
//}
//if (memcmp(playback_info_callback->play_info.str_line_1, play_info.str_line_1, sizeof(play_info.str_line_1)) != 0)
//{
//	memcpy(play_info.str_line_1, playback_info_callback->play_info.str_line_1, sizeof(play_info.str_line_1));
//	display_clear_line(hid, 0);
//	if (play_info.play_state == 1)
//	{
//		display_draw_utf8(hid, 0, 0, 0, 2, u8"▷ ");
//	}
//	else if (play_info.play_state == 0)
//	{
//		display_set_cp_user(hid);
//		display_draw_utf8(hid, 0, 0, 0, 2, u8"\x80 ");
//		display_set_cp_utf8(hid);
//		display_draw_utf8(hid, 2, 0, 0, 18, u8"已停止。");
//	}
//	else if (play_info.play_state == 3)
//	{
//		display_draw_utf8(hid, 0, 0, 0, 2, u8"▷ ");
//		display_draw_utf8(hid, 2, 0, 0, 18, u8"正在加载...");
//	}
//	if (play_info.str_line_1[0] != '\0')
//	{
//		display_draw_utf8(hid, 2, 0, 0, 18, play_info.str_line_1);
//		display_draw_utf8(hid, 0, 1, 0, 5, u8"     ");
//		display_draw_utf8(hid, 0, 1, 0, 5, playback_info_callback->play_info.str_codec);
//		snprintf(samplerate_str, sizeof(samplerate_str), "%04.1fk", playback_info_callback->play_info.samplerate / 1000.0);
//		display_draw_utf8(hid, 5, 1, 0, 6, u8"      ");
//		display_draw_utf8(hid, 5, 1, 0, 6, samplerate_str);
//	}
//	OutputDebugString(L"Update Title\r\n");
//}

// 初次使用的格式化字符串
// u8"%codec% $div(%samplerate%,1000).$div($sub(%samplerate%,$mul($div(%samplerate%,1000),1000)),100)k $padcut_right($ifgreater($div(%playback_time_seconds%,60),99,99,$div(%playback_time_seconds%,60)),2,0):$ifgreater($div(%playback_time_seconds%,60),99,++,$padcut_right($sub(%playback_time_seconds%,$mul($div(%playback_time_seconds%,60),60)),2,0))
// u8"%title%"

//在主线程内获取信息
//service_ptr_t <get_playback_info> playback_info_callback = new service_impl_t<get_playback_info>();
//static_api_ptr_t<main_thread_callback_manager> callback_manager;
//struct get_playback_info : main_thread_callback
//{
//public:
//	std::atomic<bool> thread_done;
//	const char* formatter_input_line1;
//	const char* formatter_input_line2;
//	PlayInfo play_info;
//	virtual void callback_run()
//	{
//		thread_done = false;
//		memset(&play_info, 0x00, sizeof(struct PlayInfo));
//
//		/* 使用格式化功能输出第一行自定义字符串，仅在正在播放时有效 */
//		if (formatter_input_line1 != NULL)
//		{
//			static_api_ptr_t<titleformat_compiler>()->compile_safe_ex(formatter_script, formatter_input_line1);
//			if (m_playback_control->playback_format_title(NULL, formatter_str, formatter_script, NULL, playback_control::display_level_all) == true)
//			{
//				formatter_str_len = strlen(formatter_str);
//				if (formatter_str_len >= sizeof(play_info.str_line_1))
//				{
//					memcpy(play_info.str_line_1, formatter_str, sizeof(play_info.str_line_1) - 1);
//				}
//				else
//				{
//					memcpy(play_info.str_line_1, formatter_str, formatter_str_len);
//				}
//			}
//		}
//		/* 使用格式化功能输出第二行自定义字符串，仅在正在播放时有效 */
//		if (formatter_input_line2 != NULL)
//		{
//			static_api_ptr_t<titleformat_compiler>()->compile_safe_ex(formatter_script, formatter_input_line2);
//			if (m_playback_control->playback_format_title(NULL, formatter_str, formatter_script, NULL, playback_control::display_level_all) == true)
//			{
//				formatter_str_len = strlen(formatter_str);
//				if (formatter_str_len >= sizeof(play_info.str_line_2))
//				{
//					memcpy(play_info.str_line_2, formatter_str, sizeof(play_info.str_line_2) - 1);
//				}
//				else
//				{
//					memcpy(play_info.str_line_2, formatter_str, formatter_str_len);
//				}
//			}
//		}
//		/* 获取播放状态 */
//		if (m_playback_control->get_now_playing(metadb) == true && metadb->is_info_loaded() == true)
//		{
//
//			if (m_playback_control->is_paused() == true)
//			{
//				play_info.play_state = PLAY_STATE_PAUSE; /* 暂停 */
//			}
//			else
//			{
//				play_info.play_state = PLAY_STATE_PLAY; /* 正在播放 */
//			}
//		}
//		else
//		{
//			if (m_playback_control->is_playing() == true)
//			{
//				play_info.play_state = PLAY_STATE_LOADING; /* 正在加载 */
//			}
//			else
//			{
//				play_info.play_state = PLAY_STATE_STOP; /* 停止 */
//			}
//		}
//		/* 获取音量 */
//		play_info.volume = m_playback_control->get_volume();
//		/* 线程完成 */
//		thread_done = true;
//	}
//private:
//	static_api_ptr_t<playback_control> m_playback_control;
//	titleformat_object::ptr formatter_script;
//	pfc::string_formatter formatter_str;
//	uint32_t formatter_str_len;
//	metadb_handle_ptr metadb;
//	file_info_impl file_info;
//};

		//if (update_play_state == true) /* 播放状态和第一行文字更改时更新图标和亮度 */
		//{
		//	switch (play_info->play_state)
		//	{
		//	case PLAY_STATE_STOP:
		//		display_set_cp_user(hid);
		//		display_draw_ascii(hid, 0, 0, 0, 2, ' ', "\x80");
		//		display_set_cp_utf8(hid);
		//		if (cfg_onstop_power <= 3)
		//		{
		//			display_set_power(hid, 1);
		//			display_set_dim(hid, cfg_onstop_power);
		//		}
		//		else if (cfg_onstop_power == 4)
		//		{
		//			display_set_power(hid, 1);
		//			display_clear_screen(hid);
		//			update_volume = false;
		//			update_line1 = false;
		//			update_line2 = false;
		//		}
		//		else if (cfg_onstop_power == 5)
		//		{
		//			display_set_power(hid, 0);
		//		}
		//		break;
		//	case PLAY_STATE_PLAY:
		//		display_draw_utf8(hid, 0, 0, 0, 2, ' ', u8"▷");
		//		if (cfg_onplay_power <= 3)
		//		{
		//			display_set_power(hid, 1);
		//			display_set_dim(hid, cfg_onplay_power);
		//		}
		//		else if (cfg_onplay_power == 4)
		//		{
		//			display_set_power(hid, 1);
		//			display_clear_screen(hid);
		//			update_volume = false;
		//			update_line1 = false;
		//			update_line2 = false;
		//		}
		//		else if (cfg_onplay_power == 5)
		//		{
		//			display_set_power(hid, 0);
		//		}
		//		break;
		//	case PLAY_STATE_PAUSE:
		//		display_set_cp_user(hid);
		//		display_draw_ascii(hid, 0, 0, 0, 2, ' ', "\x81");
		//		display_set_cp_utf8(hid);
		//		if (cfg_onpause_power <= 3)
		//		{
		//			display_set_power(hid, 1);
		//			display_set_dim(hid, cfg_onpause_power);
		//		}
		//		else if (cfg_onpause_power == 4)
		//		{
		//			display_set_power(hid, 1);
		//			display_clear_screen(hid);
		//			update_volume = false;
		//			update_line1 = false;
		//			update_line2 = false;
		//		}
		//		else if (cfg_onpause_power == 5)
		//		{
		//			display_set_power(hid, 0);
		//		}
		//		break;
		//	case PLAY_STATE_LOADING:
		//		display_set_cp_user(hid);
		//		display_draw_ascii(hid, 0, 0, 0, 2, ' ', "\x82");
		//		display_set_cp_utf8(hid);
		//		break;
		//	}
		//	update_play_state = false;
		//}

		//if (update_scroll == true) /* 第一行文字滚动初始化 */
		//{

		//	update_scroll = false;
		//}


		//if (play_info.play_state == PLAY_STATE_PLAY)
		//{
		//	if (scroll_delay_count < 10)
		//	{
		//		scroll_delay_count += 1;
		//	}
		//	else
		//	{
		//		scroll_delay_count = 0;
		//		scroll_pos -= 1;
		//		if (*play_info.str_line_1 != '\0')
		//		{
		//			display_scroll_utf8(play_info.str_line_1, scroll_pos, 20, scroll_str, sizeof(scroll_str));
		//			update_line1 = true;
		//		}
		//	}
		//}
		//if (update_volume == true)
		//{
		//	//snprintf(volume_str, sizeof(volume_str), "%s%06.1f%s", "  Volume: ", play_info.volume, "dB");
		//	//display_draw_utf8(hid, 0, 1, 0, 20, ' ', volume_str);
		//	update_volume = false;
		//	update_line2 = true;
		//	scroll_delay_count = 1000;
		//	OutputDebugStringA("Update Volume\n");
		//}
		//else
		//{
		//	//if (scroll_delay_count > 0 && scroll_delay_count - total_time > 0)
		//	//{
		//		//scroll_delay_count -= total_time;
		//	//}
		//	//else
		//	//{
		//		//scroll_delay_count = 0;
		//	if (update_line2 == true)
		//	{
		//		switch (play_info.play_state)
		//		{
		//		case PLAY_STATE_STOP:
		//			if (*cfg_onstop_string2 != '\0')
		//			{
		//				display_draw_utf8(hid, 0, 1, 0, 20, ' ', cfg_onstop_string2);
		//			}
		//			else
		//			{
		//				display_draw_utf8(hid, 0, 1, 0, 20, ' ', play_info.str_line_2);
		//			}
		//			break;
		//		case PLAY_STATE_PLAY:
		//			display_draw_utf8(hid, 0, 1, 0, 20, ' ', play_info.str_line_2);
		//			break;
		//		case PLAY_STATE_PAUSE:
		//			display_draw_utf8(hid, 0, 1, 0, 20, ' ', play_info.str_line_2);
		//			break;
		//		case PLAY_STATE_LOADING:
		//			if (*cfg_onload_string2 != '\0')
		//			{
		//				display_draw_utf8(hid, 0, 1, 0, 20, ' ', cfg_onload_string2);
		//			}
		//			else
		//			{
		//				display_draw_utf8(hid, 0, 1, 0, 20, ' ', play_info.str_line_2);
		//			}
		//			break;
		//		}
		//		update_line2 = false;
		//		OutputDebugStringA("Update line 2\n");
		//		//}
		//	}
		//}
		//if (update_line1 == true)
		//{
		//	scroll_delay_count = 0;
		//	switch (play_info.play_state)
		//	{
		//	case PLAY_STATE_STOP:
		//		if (*cfg_onstop_string1 != '\0')
		//		{
		//			display_draw_utf8(hid, 2, 0, 0, 18, ' ', cfg_onstop_string1);
		//		}
		//		else
		//		{
		//			display_draw_utf8(hid, 2, 0, 0, 18, ' ', scroll_str);
		//		}
		//		break;
		//	case PLAY_STATE_PLAY:
		//		display_draw_utf8(hid, 2, 0, 0, 18, ' ', scroll_str);
		//		break;
		//	case PLAY_STATE_PAUSE:
		//		display_draw_utf8(hid, 2, 0, 0, 18, ' ', scroll_str);
		//		break;
		//	case PLAY_STATE_LOADING:
		//		if (*cfg_onload_string1 != '\0')
		//		{
		//			display_draw_utf8(hid, 2, 0, 0, 18, ' ', cfg_onload_string1);
		//		}
		//		else
		//		{
		//			display_draw_utf8(hid, 2, 0, 0, 18, ' ', scroll_str);
		//		}
		//		break;
		//	}
		//	update_line1 = false;
		//	OutputDebugStringA("Update line 1\n");
		//}
		//Sleep(1);
