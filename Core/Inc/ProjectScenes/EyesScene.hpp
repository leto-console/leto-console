/**
 * @file EyesScene.hpp
 * @date May 22, 2026
 * @author Rakhimov T.
 */

#ifndef INC_PROJECT_SCENES_EYES_SCENE_HPP_
#define INC_PROJECT_SCENES_EYES_SCENE_HPP_

#include "ProjectScenes/CommonScene.hpp"
#include "ProjectScenes/SceneID.hpp"
#include <UI/Menu/ParamMenu.hpp>

#include <UI/Animation/CartoonAnimation.hpp>
#include <UI/Menu/DialogMenu.hpp>
#include <SceneManager/ISceneBuilder.hpp>

class EyesScene : public CommonScene
{
protected:
	CartoonAnimation eye1_anim;
	CartoonAnimation eye2_anim;

	void WelcomeDraw(IScreen& screen);

public:
	EyesScene();

	void OnShow() override;

	virtual void Draw(IScreen& screen) override;

	virtual bool Loop() override;

	virtual bool ProcessInput(const AppEvent& event) override;

    SCENE_NO_ARGS_BUILDER(EyesScene)
};

#endif
