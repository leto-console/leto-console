/*
 * CommonMenuScene.hpp
 *
 *  Created on: Nov 03, 2025
 *      Author: Timur
 */

#ifndef INC_PROJECT_SCENES_COMMON_MENU_SCENE_HPP_
#define INC_PROJECT_SCENES_COMMON_MENU_SCENE_HPP_

#include "ProjectScenes/CommonScene.hpp"

#include <UI/Menu/ParamMenu.hpp>

#include <Input/SystemInputID.hpp>

#include <Input/EncoderEvent.hpp>
#include <Data/StaticText.hpp>
#include <DrawFunctions/DrawText.hpp>
#include <SceneManager/ISceneManager.hpp>

class CommonMenuScene : public CommonScene
{
protected:
	ParamMenu<IScene*, 16> menu;
	IScene* currentSubScene{};
	StaticText32 title;

public:
	CommonMenuScene(ISceneManager* scene_manager, StaticText32 title = "", uint8_t menu_height = 4)
		: CommonScene{scene_manager}, menu{ menu_height, {0, title.Empty() ? 0 : 8} }, title{ title }
	{
		menu.InitBaseCatchers();
		menu.Capture();
	}

	~CommonMenuScene()
	{
		menu.Clear();
	}

	void Draw(IScreen& screen) override
	{
		if (currentSubScene)
			currentSubScene->MainDraw(screen);
		else
		{
			if (!title.Empty()) DrawFunctions::DrawText(screen, {0, 0}, title);
			menu.Draw(screen);
		}

		CommonScene::Draw(screen);
	}

	void Loop() override
	{
		CommonScene::Loop();

		if (currentSubScene)
			currentSubScene->MainLoop();
		else
			menu.Loop();
	}

	bool ProcessInput(const AppEvent& event) override
	{
		if (CommonScene::ProcessInput(event)) return true;

		if (currentSubScene)
		{
			if (currentSubScene->MainProcessInput(event))
				return true;

			// Возврат из подсцены приоритетнее
			if (IsSystemReturnEvent(event))
			{
				currentSubScene->MainOnHide();
				currentSubScene = nullptr;
				return true;
			}

			return true;
		}

		if (menu.ProcessInput(event))
			return true;

		if (IsSystemEnterEvent(event))
		{
			currentSubScene = menu.GetCurrentParam();
			menu.OnHide();
			if (currentSubScene)
				currentSubScene->MainOnShow();
			return true;
		}
		else if (IsSystemReturnEvent(event))
		{
			scene_manager->Return();
			menu.ResetCurrentID();
			return true;
		}

		return true;
	}
};


#endif