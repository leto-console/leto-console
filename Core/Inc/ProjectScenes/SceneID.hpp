/*
 * SceneID.hpp
 *
 *  Created on: Nov 03, 2025
 *      Author: Timur
 */

#ifndef INC_PROJECT_SCENES_SCENE_ID_HPP_
#define INC_PROJECT_SCENES_SCENE_ID_HPP_

enum class SceneID
{
	__SCREENS_START,
	WELCOME,			// Сцена приветствия
	EYES,				// Мини-сцена с глазками
	MAIN,				// Главное меню
	EEPROM,
	SYSTEM,
	DEBUG_SCENE,
	GAMES_CENTER,
	SETTINGS,
	SETTINGS2,
	TEST_SCENE,
	EDIT_ACCOUNT,
	DELETE_ACCOUNT,
	SETTING_ACCOUNT,
	FILE_MANAGER,
	LOGOUT,				// Сцена выхода из текущего аккаунта/режима
	__SCREENS_COUNT
};

#endif
