#include <cstdlib>
#include <iostream>
#include <openssl/sha.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <time.h>

#include "bench_common.h"

using namespace std;

static void sha1(char hash[20], const char* buf, size_t count)
{
	SHA1(reinterpret_cast<const unsigned char*>(buf), count, reinterpret_cast<unsigned char*>(hash));
}


int main(int argc, char **argv) {

    if (argc <= 1) {
        cout << "usage: ./bench_query_gen  output_filename < input_filename"
             << endl;
        exit (1);
    }

    //size_t val_len = static_cast<size_t>(-1);
    //size_t val_len = atoi(argv[1]);
    size_t key_len = NKEY;
    size_t val_len = NVAL;
    size_t num_requests = 0;

    FILE *fp = fopen(argv[1], "w");

    fwrite(&key_len, sizeof(size_t), 1, fp);
    fwrite(&val_len, sizeof(size_t), 1, fp);

    const size_t tmp_size = 1048576;
	char* tmp = new char[tmp_size];

    srand((int)time(0));

    char buf[20];
    char rawkey[1024];
    request q;
    while (fgets(tmp, tmp_size, stdin)) {
        if (sscanf(tmp, "\"recordcount\"=\"%zu\"", &num_requests)) {
            fwrite(&num_requests, sizeof(num_requests), 1, fp);
            continue;
        }if (sscanf(tmp, "INSERT usertable %s [", rawkey)) {
            q.type = request_get;
            q.delta = 0;
            sha1(buf, rawkey, strlen(rawkey));
            memcpy(q.hashed_key, buf, key_len);
        } else
            continue;

        fwrite(&q, sizeof(q), 1, fp);
    }
    delete tmp;
    fclose(fp);
}
