#include <iostream>
#include <boost/lexical_cast.hpp>

using namespace std;
int main() {
    // Example of converting a string to an integer
    std::string numberAsString = "123";
    int number = boost::lexical_cast<int>(numberAsString);

    std::cout << "Converted number: " << number << std::endl;

    return 0;
}
