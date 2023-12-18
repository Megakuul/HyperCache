#include <iostream>
#include <boost/filesystem.hpp>

using namespace std;

int main(void) {
  string text = "Hoi";

  try {
	int num = boost::lexical_cast<int>(text);
	cout << "Numsus " << num << endl;
  } catch (const boost::bad_lexical_cast &e) {
	cerr << "Err: " << e.what() << endl;
  }
  cout << "Hallo" << endl;
}
