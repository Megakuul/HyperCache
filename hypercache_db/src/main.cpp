#include "hc_db.h"
#include <vector>
#include <string>
#include <boost/asio.hpp>

int main() {
    hc_db();

    std::vector<std::string> vec;
    vec.push_back("test_package");

    hc_db_print_vector(vec);
}
