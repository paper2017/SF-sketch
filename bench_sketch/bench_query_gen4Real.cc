//#include <cstdlib>
#include <iostream>
#include <openssl/sha.h>
//#include <cstdio>
//#include <cstring>
#include <fstream>
#include <string.h>
#include <stdlib.h>
//#include <cstdint>
#include <time.h>

#include "bench_common.h"

using namespace std;

static void sha1(char hash[20], const char* buf, size_t count)
{
	SHA1(reinterpret_cast<const unsigned char*>(buf), count, reinterpret_cast<unsigned char*>(hash));
}


int main(int argc, char **argv) {

    if (argc <= 1) {
        cout << "usage: ./bench_query_gen4Real  output_filename input_filename"
             << endl;
        exit (1);
    }

    //size_t val_len = static_cast<size_t>(-1);
    //size_t val_len = atoi(argv[1]);
    size_t key_len = NKEY;
    size_t val_len = NVAL;

    size_t num_requests = 0;

    ofstream ofp(argv[1], ios::binary);

    if (!ofp) {
        cerr<< "Can't open " << argv[1] <<endl;
        abort();
    }

    ifstream ifp(argv[2]);
    if (!ifp) {
        cerr<< "Can't open " << argv[2] <<endl;
        abort();
    }
    ofp.write(reinterpret_cast<const char*>(&key_len), sizeof(size_t));
    ofp.write(reinterpret_cast<const char*>(&val_len), sizeof(size_t));
    ofp.write(reinterpret_cast<const char*>(&num_requests), sizeof(size_t));

 //   fwrite(&key_len, sizeof(size_t), 1, fp);
//    ofp.write(reinterpret_cast<const char*>(&num_requests), sizeof(size_t));
    string Prefix = "";
    string timestamp,source,aaa,bbb,ccc,ddd;
    string ipQueryStr;
    string fiveTuple;
    size_t endIndex;
    // query_t query;
    request q;
    char buf[20];
    srand((int)time(0));
	//printf("num_requests = %lu \n", num_requests);
    while (!ifp.eof()) {
        ifp >> timestamp>>source>>aaa>>Prefix>>bbb>>ccc>>ddd;
        if(Prefix.empty())
           continue;
//        fiveTuple = source + aaa + Prefix + bbb + ccc;
        fiveTuple = Prefix;
        {
            q.type = request_inc;
            q.delta = 1;
            sha1(buf, fiveTuple.c_str(), fiveTuple.length());
            memcpy(q.hashed_key, buf, key_len);
        }

        ofp.write(reinterpret_cast<const char*>(&q), sizeof(q));
        num_requests++;
		Prefix = "";
    }
	printf("num_requests = %lu \n", num_requests);
    ifp.close();
    ofp.close();

    fstream xfp(argv[1], ios::in | ios::out | ios::ate | ios::binary);

    if (!xfp) {
        cerr<< "Can't open " << argv[1] <<endl;
        abort();
    }
    xfp.seekp(2*sizeof(size_t), ofstream::beg);
    xfp.write(reinterpret_cast<const char*>(&num_requests), sizeof(size_t));
    xfp.close();
}
