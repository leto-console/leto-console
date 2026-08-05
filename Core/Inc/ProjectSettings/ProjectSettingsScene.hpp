/*
 * ProjectSettingsScene.hpp
 *
 *  Created on: Dec 12, 2025
 *      Author: Timur
 */

#ifndef INC_SETTINGS_PROJECT_SETTINGS_SCENE_HPP_
#define INC_SETTINGS_PROJECT_SETTINGS_SCENE_HPP_

#include "ProjectScenes/CommonScene.hpp"
#include <UI/Menu/ParamMenu.hpp>
#include <UI/SettingsContainer.hpp>
#include <SceneManager/ISceneBuilder.hpp>

class ProjectSettingsScene : public CommonScene
{
public:
	ProjectSettingsScene(ISceneManager* scene_manager);

	bool ProcessInput(const AppEvent& event) override;

	SCENE_NO_ARGS_BUILDER(ProjectSettingsScene)
};

#endif
