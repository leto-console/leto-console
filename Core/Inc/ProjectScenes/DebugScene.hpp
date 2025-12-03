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

class DebugScene : public CommonScene
{
public:
	void Draw(IScreen& screen) override;
	bool Loop() override { return true; };
	bool ProcessInput(const AppEvent& event) override;

	SCENE_NO_ARGS_BUILDER(DebugScene)
};

#endif