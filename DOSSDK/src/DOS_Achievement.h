#pragma once

#include <stdio.h>
#include <string>

namespace DenateAchievement{

	class DOS_Achievement
	{
    private:
        std::string userID;

        std::string appID;

    public:

        DOS_Achievement(std::string userID, std::string appID);

        // Returns a + b
        double Add(double a, double b);

        // Returns a - b
        static double Subtract(double a, double b);

        // Returns a * b
        static double Multiply(double a, double b);

        // Returns a / b
        static double Divide(double a, double b);
	};

}