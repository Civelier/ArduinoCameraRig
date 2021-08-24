// LogicTests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <stdexcept>

enum MicroStep
{
	MSStep = 1,
	MSHalf = 2,
	MSQuarter = 3,
	MSEighth = 8,
	MSSixteenth = 16,
};

int stepsLeft;

int StepFactor(MicroStep ms)
{
	switch (ms)
	{
	case MSStep:
		return 16;
	case MSHalf:
		return 8;
	case MSQuarter:
		return 4;
	case MSEighth:
		return 2;
	case MSSixteenth:
		return 1;
	default:
		return 0;
	}
}

MicroStep ReadStepSize()
{
	while (true)
	{
		try
		{
			int input;
			std::cin >> input;

			switch (input)
			{
			case MSStep:
			case MSHalf:
			case MSQuarter:
			case MSEighth:
			case MSSixteenth:
				return static_cast<MicroStep>(input);
			default:
				break;
			}
		}
		catch (std::exception e)
		{
		}
	}
}

void Step(MicroStep stepSize)
{
	stepsLeft -= StepFactor(stepSize);
	std::cout << stepsLeft << " - stepped 1/" << stepSize;
}

void StepUp(MicroStep ms)
{

	for (size_t i = 4; i > 0; i--)
	{

	}
}

int main()
{
	std::cout << "Enter the number of steps left: ";
	std::cin >> stepsLeft;
	
	std::cout << "Enter next step size: ";
	auto current = ReadStepSize();

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
