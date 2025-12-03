/*
 * MainApp.cpp
 *
 *  Created on: Feb 11, 2025
 *      Author: Timur
 */

#ifndef USE_HAL_DRIVER

#include "Application/System.hpp"

int main(int argc, char* argv[])
{
	bool reboot = false;
	do
	{
		Application app{ argc, argv };
		app.Init();

		while (app.Loop())
		{
		}

		reboot = app.GetRebootFlag();
	} 
	while (reboot);
	return 0;
}

#endif
