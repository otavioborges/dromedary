#include <iostream>
#include "sftp.h"
#include "sftpFunctions.h"
#include "fuseFs.h"
using namespace std;
using namespace dromedary;

int main(int argc, char const *argv[]){
    char desmanche;

    Sftp manager("www.oleivas.com.br", 22);
    manager.LoadKey("src/id_rsa");
    manager.Connect("otavio");

    // SftpFunctions functions(&manager);
    // functions.List("/storage");

    InitFuse(&manager);
    Mount("/home/otavio/fuse");
    cin >> desmanche;
    Unmount();

    return 0;
}
