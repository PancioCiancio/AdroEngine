#include "VkApp.h"

int main()
{
	VkApp app = {};
	app.Init();
	app.Update();
	app.TearDown();

	return 0;
}