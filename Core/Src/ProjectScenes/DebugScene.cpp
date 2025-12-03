#include "ProjectScenes/DebugScene.hpp"
#include <Input/SystemInputID.hpp>

#include "Common/ProjectGraphs.hpp"

#include <SSD1306/SSD1306_Properties.hpp>

#include <Utils/crc16.hpp>
#include "ProjectScenes/CreateCharInfo.hpp"

#include <DrawFunctions/DrawText.hpp>

void DebugScene::Draw(IScreen& screen)
{
	static StaticText8 clk_text = "CLK";
	static StaticText8 dt_text = "DT";

	DrawFunctions::DrawText(screen, {0, 0}, clk_text);
	DEBUG_ENCODER_CLK.Draw(screen, {0, 0});
	DrawFunctions::DrawText(screen, {0, SSD1306_Height / 2}, dt_text);
	DEBUG_ENCODER_DT.Draw(screen, {0, SSD1306_Height / 2});

	//char text[256];
	//uint32_t data = 0xC1A0BABE;
	//snprintf(text, 256, "%04X", calc_crc16(&data, sizeof(data)));
	//
	//screen.SimpleText(50, 0, text);
}

bool DebugScene::ProcessInput(const AppEvent& event)
{
	if (IsSystemReturnEvent(event))
		SceneManager::Instance().Return();
	return true;
}
