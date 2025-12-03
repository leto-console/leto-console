#include "ProjectScenes/EyesScene.hpp"

#include "ProjectScenes/SceneID.hpp"

#include <Input/SystemInputID.hpp>

#include <Input/ButtonEvent.hpp>
#include <SceneManager/SceneManager.hpp>
#include <Time/Timer.hpp>

#include <Bitmaps/Eyes.hpp>

#include <Data/StaticList.hpp>
#include <DrawFunctions/DrawBitmap.hpp>

// ====================================================================================================

void EyesScene::WelcomeDraw(IScreen& screen)
{
	using namespace DrawFunctions;
	eye1_anim.Tick();
	eye2_anim.Tick();

	const BitmapData* frame1 = eye1_anim.GetCurrent();
	if (frame1)
		DrawBitmap(screen, {64 - frame1->width, 32 - frame1->height / 2}, *frame1, WhiteColor);

	const BitmapData* frame2 = eye2_anim.GetCurrent();
	if (frame2)
		DrawBitmap(screen, {64, 32 - frame2->height / 2}, *frame2, WhiteColor);
}

#include <System/SystemMode.hpp>

EyesScene::EyesScene() :
	eye1_anim(GetEyeAnim(true), 80),
	eye2_anim(GetEyeAnim(false), 80)
{
}

void EyesScene::OnShow()
{
    eye1_anim.Play();
    eye2_anim.Play();
}

void EyesScene::Draw(IScreen& screen)
{
	CommonScene::Draw(screen);

	if (eye1_anim.IsPlaying() || eye2_anim.IsPlaying())
	{
		WelcomeDraw(screen);
	}
}

bool EyesScene::Loop()
{
	CommonScene::Loop();

	if (!eye1_anim.IsPlaying() && !eye2_anim.IsPlaying())
        SceneManager::Instance().SwitchScene(SceneID::MAIN);

	return true;
}

bool EyesScene::ProcessInput(const AppEvent& event)
{    
	return true;
}

