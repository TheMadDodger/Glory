#include "TestCommandHandler.h"
#include "Console.h"

namespace Glory
{
	void TestCommandHandler::OnInvoke(TestCommand commandData)
	{
		Console::WriteLine("TestCommand: Hello World! " + commandData.Text);
	}
}
