#pragma once

#include "stdafx.h"
#include <Windows.h>
#include <atomic>

struct PlayInfo
{
	char str_line_1[301]; /* 第一行自定义格式化字符串，最多 100 个 3Byte UTF8 字符*/
	std::atomic<bool> str_line1_avaliable;
	std::atomic<bool> update_str_line1;
	char str_line_2[301]; /* 第二行自定义格式化字符串，最多 100 个 3Byte UTF8 字符*/
	std::atomic<bool> str_line2_avaliable;
	std::atomic<bool> update_str_line2;
	std::atomic<bool> is_new_track;
	std::atomic<char> play_state;
	std::atomic<bool> update_play_state;
	std::atomic<double> volume;
	std::atomic<bool> update_volume;
};

struct DisplayThreadData
{
	unsigned short vid;
	unsigned short pid;
	std::atomic<int> status;
	std::atomic<int> stop;
	struct PlayInfo play_info;
};

class DisplayThread :public play_callback
{
public:
	DisplayThreadData thread_data;
	int start(unsigned short vid, unsigned short pid);
	int stop(void);
	void format_line1(const char* format);
	void format_line2(const char* format);
	virtual void on_playback_starting(play_control::t_track_command p_command, bool p_paused);
	virtual void on_playback_new_track(metadb_handle_ptr p_track);
	virtual void on_playback_stop(play_control::t_stop_reason p_reason);
	virtual void on_playback_seek(double p_time);
	virtual void on_playback_pause(bool p_state);
	virtual void on_playback_edited(metadb_handle_ptr p_track);
	virtual void on_playback_dynamic_info(const file_info& p_info) {}
	virtual void on_playback_dynamic_info_track(const file_info& p_info) {}
	virtual void on_playback_time(double p_time);
	virtual void on_volume_change(float p_new_val);
	virtual unsigned int get_flags(void);
private:
	HANDLE thread_handle;
};
