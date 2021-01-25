#include<stdlib.h>
//#include<string.h>


using i32 = uint32_t;

extern "C" i32 print_num( i32 n );
extern "C" void print( const char * n );
extern "C" i32 new_address_map();
extern "C" void set_address_map(i32 map_id, i32 key, i32 val);
extern "C" i32 get_address_map(i32 map_id, i32 val);
extern "C" void consoleTime();
extern "C" void consoleTimeEnd();
extern "C" i32 timeSeed();
extern "C" void* memory;


extern "C" {

    int calling2(int p) {
        print("{\"isTestResult\":true, \"ok\": false}");
        return 330 + p;
    }

    int calling(int p) {
        print("time seed = ");
        //print_num(timeSeed());
        //srand(timeSeed());

        //srand((unsigned int)time(NULL));
        

        char str1[128] = {1,1,1,1,5,1,1};
        char str2[] = "ab\0cde";
        int i;

        for (i=0; i<7; i++) {
            print_num(str1[i]);
        }

        print("\n");

        //memcpy(str1, str2, 5);

        //clock_t start = clock();

        uint64_t loopCount = 1000 * 10 * 1000LLU;
        uint64_t current = 255;

        uintptr_t val;

        //i32 map_id = new_address_map();

        {
            for (int i = 0; i < 1000*1000; i++) {
                //print_num(580032*i);
                if (i > 55) {
                    /*
                       A * a = new A();//aobj{};
                       val = i;
                       set_address_map(map_id, i, (uintptr_t)a);
                       delete a;
                       */
                    //val = get_address_map(map_id, i);
                    //vec.push_back(3);
                }
            }
        }


        /*
            char * ch = new char[]{"haoiwjee2"};
            print(ch);
            print("______joifwe");

            val = get_address_map(map_id, 1000);
            print_num(val);

            char *result = static_cast<char*>(malloc(1024));
            A * a = new A();//aobj{};
            //strncpy(result, "jifow", strlen("jifow"));

            print_num((uintptr_t)a);
            delete a;
        */


        /*
           print_num((int)result);
           i32 map_id = new_address_map();
           set_address_map(map_id, 155, (uintptr_t)a);
           uintptr_t val = get_address_map(map_id, 155);
           A* getP = (A*)val;
           getP->g;
           print_num(getP->g);
           delete a;

        //std::unordered_map<int, int> um;
        //um.insert(std::pair<int,int>{3,5});
        */

        print_num(2939);

        return 255;

     //return *aaa;//a->func();//*ii;;
    }
}

