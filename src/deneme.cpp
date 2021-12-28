#include <iostream>
#include <sstream>
using namespace std;
 
int main()
{
    string s = "12345 12";
 
    // object from the class stringstream
    stringstream geek(s);
 
    // The object has the value 12345 and stream
    // it to the integer x
    int x = 0;
    int y = 1;
    geek >> x >> y;
    
 
    // Now the variable x holds the value 12345
    cout << "Value of y : " << x;
 
    return 0;
}