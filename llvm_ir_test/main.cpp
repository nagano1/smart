#include <cstdio>
#include "lib.h"

//#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
//#include <windows.h>
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   //LPSTR lpszCmdLine, int nCmdShow) {
    //MessageBox(NULL, TEXT("Hello, world!"), TEXT("Hello"), MB_OK | MB_ICONINFORMATION);
//#else
int main(int argc, char **argv) {
//#endif

    int i = 242;
    printf("Hello, world!%d \n", i);
    func(4);
    return 0;
}
