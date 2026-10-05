/*
 * DebugScene.hpp
 *
 *  Created on: Nov 08, 2025
 *      Author: Timur
 */

#ifndef INC_PROJECT_SCENES_DEBUG_SCENE_HPP_
#define INC_PROJECT_SCENES_DEBUG_SCENE_HPP_

#include "ProjectScenes/CommonScene.hpp"
#include "ProjectScenes/SceneID.hpp"

#include <SceneManager/ISceneBuilder.hpp>
#include <Input/Catchers/ButtonCatcher.hpp>

#include <Input/SystemInputID.hpp>

class DebugScene : public CommonScene
{
protected:
	ButtonCatcher<DebugScene> multy_test;
	unsigned multy{ };
	unsigned single{ };

public:
	DebugScene(ISceneManager* scene_manager) 
		: CommonScene{scene_manager}, multy_test{this, &DebugScene::Multy}
	{
		multy_test.Enable();
		multy_test.Catch(SYSTEM_BTN_UP, BCM_HOLD_MULTIPLY);
		multy_test.Catch(SYSTEM_BTN_DOWN, BCM_HOLD_MULTIPLY);
		multy_test.SetHoldTime(3000);
		AddObject(&multy_test);
	}

	void Multy()
	{
		multy++;
	}

	void Draw(IScreen& screen) override;
	void Loop() override { };
	bool ProcessInput(const AppEvent& event) override;

	SCENE_NO_ARGS_BUILDER(DebugScene)
};

#endif