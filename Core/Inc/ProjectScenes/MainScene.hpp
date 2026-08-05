/*
 * MainScene.hpp
 *
 *  Created on: Nov 04, 2025
 *      Author: Timur
 */

#ifndef INC_PROJECT_SCENES_MAIN_SCENE_HPP_
#define INC_PROJECT_SCENES_MAIN_SCENE_HPP_

#include "ProjectScenes/CommonScene.hpp"
#include "ProjectScenes/SceneID.hpp"
#include <UI/Menu/ParamMenu.hpp>

#include <UI/Animation/CartoonAnimation.hpp>
#include <UI/Menu/DialogMenu.hpp>
#include <SceneManager/ISceneBuilder.hpp>

class MainScene : public CommonScene
{
protected:
	ParamMenu<SceneID, 8> menu;
	DialogParamMenu<bool, 8> exit_question;

public:
	MainScene(ISceneManager* scene_manager);

	virtual bool Loop() override;

	virtual bool ProcessInput(const AppEvent& event) override;

	SCENE_NO_ARGS_BUILDER(MainScene)
};

#endif
