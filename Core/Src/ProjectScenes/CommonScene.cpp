#include "ProjectScenes/CommonScene.hpp"

StaticList<CommonScene::PrecipitationItem, 64> CommonScene::precipitation_list;
Timer CommonScene::precipitation_timer;

CommonScene::CommonScene(ISceneManager* scene_manager) : IScene{scene_manager}
{
}

void CommonScene::CheckModeChanged()
{
	if (mode != prev_mode)
	{
		for (int i = 0; i < _MAX_SPEED; ++i)
		{
			int denum = 1;
			
			if (mode == MODE::_SNOW)
			{
				if		(i == _16) denum = 16;
				else if (i == _32) denum = 32;
				else if (i == _48) denum = 48;
			}
			else if (mode == MODE::_RAIN)
			{
				if		(i == _16) denum = 30 * 4;
				else if (i == _32) denum = 40 * 5;
				else if (i == _48) denum = 50 * 5;
			}

			speed_timers[i].Start(1000 / denum);
		}

		prev_mode = mode;
	}
}

static size_t screen_width, screen_height;

#include <DrawFunctions/DrawLine.hpp>

void CommonScene::Draw(IScreen& screen)
{
	if (!screen_width || !screen_height)
	{
		screen_width = screen.Width();
		screen_height = screen.Height();
	}

	if (!precipitation_list.empty())
	{
		for (PrecipitationItem& item : precipitation_list)
		{
			if (mode == MODE::_SNOW)
				screen.PixelSet({item.col, item.row}, WhiteColor);
			else if (mode == MODE::_RAIN)
				DrawFunctions::DrawLine(screen, {item.col, item.row}, {item.col, item.row - 3}, BlueColor);
		}
	}
};

#include <algorithm>

static bool RemovePred(CommonScene::PrecipitationItem& item)
{
	return item.row > screen_height;
}

void CommonScene::Loop()
{
	CheckModeChanged();

	if (!screen_width || !screen_height)
		return;

	if (EnableSnowfall.GetOrDefault())
	{
		if (precipitation_timer.Expired())
		{
			if (precipitation_list.size() < 64)
			{
				PrecipitationItem item{ 0, Random<short>(0, screen_width) };
				item.speed_type = Random(0, 3);
				precipitation_list.push_back(item);
			}
			
			if (mode == MODE::_SNOW)
				precipitation_timer.Start(Random<uint32_t>(100, 300));
			else if (mode == MODE::_RAIN)
				precipitation_timer.Start(Random<uint32_t>(30, 100));
		}
	}

	bool speed_timer_expired[_MAX_SPEED]{};
	
	for (int i = 0; i < _MAX_SPEED; ++i)
	{
		if (speed_timers[i].Expired())
		{
			speed_timer_expired[i] = true;
			speed_timers[i].Start();
		}
	}

	if (!precipitation_list.empty())
	{
		for (PrecipitationItem& item : precipitation_list)
		{
			if (speed_timer_expired[item.speed_type])
			{
				item.row += 1;
			}
		}

		precipitation_list.RemoveIf(&RemovePred);
	}
}

