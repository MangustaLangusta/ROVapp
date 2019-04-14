
#include "windows.h"
#include <iostream>
#include "ControlPad.h"
using namespace std;

int main() {
    COM_init();
    cout<<"Ready to recieve. How many bytes do you want?"<<endl;
    int ready;
    cin>>ready;
    cout<<endl;
    if (ready)
    {
        ReadCOM(ready);
        cout<<"finita"<<endl;
    }
    
    return 0;
}

