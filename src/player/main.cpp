// #include "hello_triangle.h"
#include "hello_application.h"
#include <iostream>

int main()
{
	// PlayerApplication::HelloTriangleApplication app;

	try
	{
		PlayerApplication::HelloApplication app;
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
