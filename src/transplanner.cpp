
#include <string>
#include <vector>
#include <iostream>

#include

using std::cout;
using std::endl;

//  This is really done in Process Commands,

int main(int argc, int argv[])
{
    return 0;
}

int ParseArgs(const std::vector<std::string> &args, std::string &datapath, std::string &resultspath)
{
    auto StdIn = std::make_shared <
                 for (auto &Argument : args)
    {
        if (Argument.find("--data=") == 0)
        {
            datapath = Argument.substr(7);
        }
    }

    std::cout << DataPath << " " << ResultsPath << endl;
    auto StdIn = std::make_shared < CStandardDataSource
}