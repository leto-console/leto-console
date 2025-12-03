/*
 * CreateCharInfo.hpp
 *
 *  Created on: Nov 03, 2025
 *      Author: Timur
 */

#ifndef INC_EEPROM_SCENE_HPP_
#define INC_EEPROM_SCENE_HPP_

#include "ProjectScenes/CommonMenuScene.hpp"

#include <UI/Menu/Menu.hpp>

#include <SceneManager/IScene.hpp>
#include <EEPROM/EEPROM_24C.hpp>
#include <SceneManager/ISceneBuilder.hpp>

class EEPROMScene : public CommonMenuScene
{
public:
	EEPROMScene(EEPROM_24C* eeprom);

	class Builder : public ISceneBuilder
	{
	public:
		Builder(EEPROM_24C* eeprom) : eeprom{ eeprom } { }
	protected:
		EEPROM_24C* eeprom;

		IScene* Create(IAllocator& allocator) override
		{
			return allocator.Make<EEPROMScene>(eeprom);
		}
	};
};

#endif
