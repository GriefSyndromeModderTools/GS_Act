#include "act_utils.h"

//wrappers for windows function
#include <Windows.h>

void Act::Platform::ErrorMessageBox(std::string msg)
{
	::MessageBoxA(NULL, msg.c_str(), "Error", 0);
}
