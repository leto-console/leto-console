/*
 * WelcomeScene.hpp
 *
 *  Created on: Mar 02, 2026
 *      Author: Timur
 */

#ifndef INC_PROJECT_SCENES_WELCOME_SCENE_HPP_
#define INC_PROJECT_SCENES_WELCOME_SCENE_HPP_

#include "ProjectScenes/CommonScene.hpp"
#include "ProjectScenes/SceneID.hpp"
#include <UI/Menu/ParamMenu.hpp>

#include <UI/Animation/Animation.hpp>
#include <UI/Menu/DialogMenu.hpp>
#include <Time/Timer.hpp>
#include <Data/StaticText.hpp>

#include <SceneManager/ISceneBuilder.hpp>

class WelcomeScene : public CommonScene
{
protected:
	StaticText<8> name;
	Timer timer;

public:
	WelcomeScene();

	void OnShow() override;

	virtual void Draw(IScreen& screen) override;

	virtual bool Loop() override;

	virtual bool ProcessInput(const AppEvent& event) override;

	SCENE_NO_ARGS_BUILDER(WelcomeScene)
};

#endif
