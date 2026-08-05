/*
 * SystemScene.hpp
 *
 *  Created on: Nov 08, 2025
 *      Author: Timur
 */

#ifndef INC_PROJECT_SCENES_SYSTEM_SCENE_HPP_
#define INC_PROJECT_SCENES_SYSTEM_SCENE_HPP_

#include "ProjectScenes/CommonMenuScene.hpp"

#include <SceneManager/ISceneBuilder.hpp>

class SystemScene : public CommonMenuScene
{
public:
	SystemScene(ISceneManager* scene_manager);

	SCENE_NO_ARGS_BUILDER(SystemScene)
};

#endif
