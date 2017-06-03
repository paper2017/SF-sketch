#include <cstdlib>
#include <iostream>
#include <openssl/sha.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <time.h>

#include "stdio.h"
#include "string.h"
#include "city.h"
#include "data4hash.h"

using namespace std;

static void sha1(char hash[20], const char* buf, size_t count)
{
	SHA1(reinterpret_cast<const unsigned char*>(buf), count, reinterpret_cast<unsigned char*>(hash));
}


int main(int argc, char **argv) {

    if (argc <= 2) {
        cout << "usage: ./data4hash [a|b|c]  output_filename < input_filename"
             << endl;
        exit (1);
    }

    //size_t val_len = static_cast<size_t>(-1);
    //size_t val_len = atoi(argv[1]);
    size_t key_len;
    size_t val_len = sizeof(int32);
    size_t num_requests = 0;

    FILE *fp = fopen(argv[2], "w");

    const size_t tmp_size = 1048576;
	char* tmp = new char[tmp_size];

    srand((int)time(0));

    char buf[20];
    char rawkey[1024];
    char value[20];
    requestA ra;
    requestB rb;
    requestC rc;

    uint64 ka;
    uint128 kb;

    printf("key type: %s \n", argv[1]);
    if (argv[1][0] == 'a') {
        key_len = 8;
//        fwrite(&key_len, sizeof(size_t), 1, fp);
//        fwrite(&val_len, sizeof(size_t), 1, fp);

        while (fgets(tmp, tmp_size, stdin)) {
            if (sscanf(tmp, "\"recordcount\"=\"%zu\"", &num_requests)) {
//                fwrite(&num_requests, sizeof(num_requests), 1, fp);
                continue;
            }if (sscanf(tmp, "INSERT usertable %s [ field0=%4s", rawkey, value)) {
                ra.value = stol(value);
                ka = CityHash64(rawkey, strlen(rawkey));
                lltos(ra.key, ka);
            } else
            continue;

            toRechar(ra.key, key_len);
            fprintf(fp, "%.8s %i\n", ra.key, ra.value);
//            fwrite(&ra, sizeof(ra), 1, fp);
        }
    } else if (argv[1][0] == 'b') {
        key_len = 16;
//        fwrite(&key_len, sizeof(size_t), 1, fp);
//        fwrite(&val_len, sizeof(size_t), 1, fp);
        while (fgets(tmp, tmp_size, stdin)) {
            if (sscanf(tmp, "\"recordcount\"=\"%zu\"", &num_requests)) {
//                fwrite(&num_requests, sizeof(num_requests), 1, fp);
                continue;
            }if (sscanf(tmp, "INSERT usertable %s [ field0=%4s", rawkey, value)) {
                rb.value = stol(value);
                kb = CityHash128(rawkey, strlen(rawkey));
                lltos(rb.key, Uint128Low64(kb));
                lltos(&rb.key[8], Uint128High64(kb));
            } else

            continue;

            toRechar(rb.key, key_len);
            fprintf(fp, "%.16s %i\n", rb.key, rb.value);

//            fwrite(&rb, sizeof(rb), 1, fp);
        }
    } else if (argv[1][0] == 'c') {
        key_len = 128;
//        fwrite(&key_len, sizeof(size_t), 1, fp);
//        fwrite(&val_len, sizeof(size_t), 1, fp);
        while (fgets(tmp, tmp_size, stdin)) {
            if (sscanf(tmp, "\"recordcount\"=\"%zu\"", &num_requests)) {
//                fwrite(&num_requests, sizeof(num_requests), 1, fp);
                continue;
            }if (sscanf(tmp, "INSERT usertable %s [ field0=%4s", rawkey, value)) {
                rc.value = stol(value);
                memcpy(rc.key, rawkey, strlen(rawkey) > 128 ? 128 : strlen(rawkey));
            } else
            continue;

            toRechar(rc.key, key_len);
            fprintf(fp, "%.128s %i\n", rc.key, rc.value);
//            fwrite(&rc, sizeof(rc), 1, fp);
        }
    }

    delete tmp;
    fclose(fp);
}
