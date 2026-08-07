/*
 * CommonScene.hpp
 *
 *  Created on: Nov 03, 2025
 *      Author: Timur
 */

#ifndef INC_PROJECT_SCENES_COMMON_SCENE_HPP_
#define INC_PROJECT_SCENES_COMMON_SCENE_HPP_

#include <SceneManager/IScene.hpp>
#include <Input/ButtonEvent.hpp>

#include "ProjectScenes/SceneID.hpp"

#include "ProjectSettings/ProjectSettings.hpp"
#include <Utils/random.hpp>
#include <Time/Timer.hpp>
#include <Data/StaticList.hpp>

class CommonScene : public IScene
{
public:
	struct PrecipitationItem
	{
		short row, col;
		char speed_type;
	};

protected:
	enum SPEED : char
	{
		_16,
		_32,
		_48,
		_MAX_SPEED
	};
	Timer speed_timers[_MAX_SPEED];

	enum MODE : char
	{
		_UNKNOWN,
		_SNOW,
		_RAIN,
		_MAX_MODE
	};
	MODE prev_mode 	= MODE::_UNKNOWN;
	MODE mode 		= MODE::_SNOW;

	static StaticList<PrecipitationItem, 64> precipitation_list;
	static Timer precipitation_timer;

	void CheckModeChanged();

public:
	CommonScene(ISceneManager* scene_manager);
	virtual void Draw(IScreen& screen) override;
	virtual void Loop() override;
	virtual bool ProcessInput(const AppEvent& event) override { return false; };
};

#endif