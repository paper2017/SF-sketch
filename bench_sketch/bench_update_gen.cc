#include <getopt.h>
#include <cstdlib>
#include <iostream>
#include <openssl/sha.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <time.h>
#include <string.h>

#include <assert.h>
#include <libmemcached/memcached.h>
#include "bench_common.h"

using namespace std;

static void sha1(char hash[20], const char* buf, size_t count)
{
    SHA1(reinterpret_cast<const unsigned char*>(buf), count, reinterpret_cast<unsigned char*>(hash));
}


/* default parameter settings */
static size_t key_len = NKEY;
static size_t val_len = NVAL;
static size_t num_requests = 0;
static size_t num_records = 0;

/* create a memcached structure */
static memcached_st *memc_new()
{
    char config_string[1024];
    memcached_st *memc = NULL;
    unsigned long long getter;

    sprintf(config_string, "--SERVER=%s --BINARY-PROTOCOL", "127.0.0.1");
    printf("config_string = %s\n", config_string);
    memc = memcached(config_string, strlen(config_string));

    getter = memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_NO_BLOCK);
    printf("No block: %lld\n", getter);
    getter = memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE);
    printf("Socket send size: %lld\n", getter);
    getter = memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE);
    printf("Socket recv size: %lld\n", getter);

    return memc;
}

/* wrapper of set command */
static int memc_put(memcached_st *memc, char *key, char *val) {
    memcached_return_t rc;
    rc = memcached_set(memc, key, key_len, val, val_len, (time_t) 0, (uint32_t) 0);
    if (rc != MEMCACHED_SUCCESS) {
        return 1;
    }
    return 0;
}

/* wrapper of get command */
static char* memc_get(memcached_st *memc, char *key) {
    memcached_return_t rc;
    char *val;
    size_t len;
    uint32_t flag;
    val = memcached_get(memc, key, key_len, &len, &flag, &rc);
    if (rc != MEMCACHED_SUCCESS) {
        return NULL;
    }
    return val;
}


/* wrapper of set command */
static int memc_init_inc(memcached_st *memc, char *key, uint64_t offset, uint64_t initial, uint64_t *value) {
    memcached_return_t rc;
    rc = memcached_increment_with_initial(memc, key, key_len, offset, initial, (time_t) 0, value);
    if (rc != MEMCACHED_SUCCESS) {
        return 1;
    }
    return 0;
}

/* wrapper of set command */
static int memc_inc(memcached_st *memc, char *key, uint32_t offset, uint64_t *value) {
    memcached_return_t rc;
    rc = memcached_increment(memc, key, key_len, offset, value);
    if (rc != MEMCACHED_SUCCESS) {
        return 1;
    }
    return 0;
}

/* wrapper of set command */
static int memc_dec(memcached_st *memc, char *key, uint32_t offset, uint64_t *value) {
    memcached_return_t rc;
    rc = memcached_decrement(memc, key, key_len, offset, value);
    if (rc != MEMCACHED_SUCCESS) {
        return 1;
    }
    return 0;
}




#ifdef UNIFORM_OPERATE
/* init all requests from the ycsb trace file before issuing them */
static request *requests_init(char* filename)
{
    FILE *input;

    input = fopen(filename, "rb");
    if (input == NULL) {
        perror("can not open file");
        perror(filename);
        exit(1);
    }

    int n;
    size_t num_requests_tmp;
    n = fread(&key_len, sizeof(key_len), 1, input);
    if (n != 1)
        perror("fread error");
    n = fread(&val_len, sizeof(val_len), 1, input);
    if (n != 1)
        perror("fread error");
    n = fread(&num_requests_tmp, sizeof(num_requests_tmp), 1, input);
    if (n != 1)
        perror("fread error");

    if (num_requests_tmp != num_records) {
        perror("input_file and check_file have different num_records!!! exit!\n");
        exit(-1);
    }

    printf("check uniform frequent(%s):\n", filename);
    printf("\tkey_len = %zu\n", key_len);
    printf("\tval_len = %zu\n", val_len);
    printf("\tnum_requests = %zu\n", num_requests_tmp);
    printf("\n");

    request *requests = (request *)malloc(sizeof(request) * num_requests);
    if (requests == NULL) {
        perror("not enough memory to init requests\n");
        exit(-1);
    }

    size_t num_read;
    num_read = fread(requests, sizeof(request), num_requests_tmp, input);
    if (num_read < num_requests_tmp) {
        fprintf(stderr, "num_read: %zu\n", num_read);
        perror("can not read all requests\n");
        fclose(input);
        exit(-1);
    }

    fclose(input);
    printf("requests_init...done\n");
    return requests;
}
#endif









static void usage(char* binname)
{   //"usage: ./bench_update_gen  output_filename input_filename check_filename append_or_not[yes/no]"
#ifdef UNIFORM_OPERATE
    printf("%s [-o output_filename] [-i input_filename] [-c check_filename] [-a] [-d] [-l] [-h]\n", binname);
    printf("\t-c check_filename: use check\n");
#else
    printf("%s [-o output_filename] [-i input_filename] [-a] [-d]  [-h]\n", binname);
#endif
    printf("\t-o output_filename: indicate output file name, required\n");
    printf("\t-i input_filename: indicate input file name, required\n");
    printf("\t-a : append to output file\n");
    printf("\t-d : decrease only\n");
    printf("\t-l : we use load file generated by YCSB as input file\n");
    printf("\t-h : show usage\n");
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        usage(argv[0]);
        exit(-1);
    }

    FILE *fp;
    FILE *input;
    bool isappend = false;
    bool decreaseonly = false;
    bool isload = false;
    char *output_filename = NULL;
    char *input_filename = NULL;
#ifdef UNIFORM_OPERATE
    char *check_filename = NULL;
#endif

    char ch;
    while ((ch = getopt(argc, argv,
                        "o:"
                        "i:"
#ifdef UNIFORM_OPERATE
                        "c:"
#endif
                        "a"
                        "d"
                        "l"
                        "h"
                       )) != -1) {
        switch (ch) {
        case 'o':
            output_filename = optarg;
            break;
        case 'i':
            input_filename = optarg;
            break;
#ifdef UNIFORM_OPERATE
        case 'c':
            check_filename = optarg;
            break;
#endif
        case 'a':
            isappend = true;
            break;
        case 'd':
            decreaseonly = true;
            break;
        case 'l':
            isload = true;
            break;
        case 'h':
            usage(argv[0]);
            exit(0);
            break;
        default:
            usage(argv[0]);
            exit(-1);
        }
    }

    if (output_filename == NULL || input_filename == NULL ) {
        usage(argv[0]);
        exit(-1);
    }

    if (isappend == true) {
        fp = fopen(output_filename, "a");
        printf("append file %s\n", output_filename);
    } else {
        fp = fopen(output_filename, "w");
    }
    input = fopen(input_filename, "r");

    if (fp == NULL || input == NULL) {
        perror("can not open file/s");
        exit(1);
    }
    if (!isappend) {
        fwrite(&key_len, sizeof(size_t), 1, fp);
        fwrite(&val_len, sizeof(size_t), 1, fp);
    }

    const size_t tmp_size = 1048576;
    char* tmp = new char[tmp_size];

    srand((int)time(0));

    memcached_st *memc;
    memc = memc_new();
    while (fgets(tmp, tmp_size, input)) {
        char rawkey[1024];
        if (sscanf(tmp, "\"recordcount\"=\"%zu\"", &num_records)) {
            if (isload && !isappend) {
                fwrite(&num_records, sizeof(num_records), 1, fp);
                break;
            }
        } else if (sscanf(tmp, "\"operationcount\"=\"%zu\"", &num_requests)) {
            if (!isload && !isappend) {
                fwrite(&num_requests, sizeof(num_requests), 1, fp);
            }
            break;
        }
    }

#ifdef UNIFORM_OPERATE
    if (isload == false && (num_requests == 0 || num_records == 0)) {
        printf("INPUT FILE is not well defined! exit!\n");
        exit(1);
    }
    int frequent = num_requests / num_records;
    if (!isload)
        printf("The UNIFORM FREQUENT is %d\n", frequent);
#endif

    while (fgets(tmp, tmp_size, input)) {
        char buf[20];
        char rawkey[1024];
        request q;
        int ret;
        uint64_t value;
#ifdef ONLY_ONE_OPERATE
        size_t delta = 1;
#else
        size_t delta = ((double)rand() / ((double)RAND_MAX+1)) * GAP_DELTA + 1;
#endif
        if (sscanf(tmp, "INSERT usertable %s [ field", rawkey)) {
            q.type = request_init;
            delta = 0;
            q.delta = delta;
            sha1(buf, rawkey, strlen(rawkey));
            memcpy(q.hashed_key, buf, key_len);
            ret = memc_init_inc(memc, q.hashed_key, 0, delta, &value);
        }
        else if (sscanf(tmp, "READ usertable %s [", rawkey)) {
            sha1(buf, rawkey, strlen(rawkey));
            memcpy(q.hashed_key, buf, key_len);
            if (!decreaseonly && (delta > ZERO_DELTA)) {
                delta -= ZERO_DELTA;
                q.type = request_inc;
#ifdef UNIFORM_OPERATE
                ret = memc_inc(memc, q.hashed_key, 0, &value);
                if (ret == 0) {
                    if (value < frequent) {
                        ret = memc_inc(memc, q.hashed_key, delta, &value);
                    } else {
                        continue;
                    }
                } else {
                    continue;
                }
#else
                ret = memc_inc(memc, q.hashed_key, delta, &value);
#endif
            }
#ifndef FC_V1
            else {
                q.type = request_dec;
                ret = memc_inc(memc, q.hashed_key, 0, &value);
                if (ret == 1) {
                    //delta = 0;//didn't exist this hashed_key
                    continue;
                } else {
#ifdef ONLY_ONE_OPERATE
                    delta = 1;
                    if (value < delta) {
                        printf("A");
                        continue;
                    }
#else
                    delta = ((double)rand() / ((double)RAND_MAX+1)) * value;
#endif
                }
                ret = memc_dec(memc, q.hashed_key, delta, &value);
            }
#endif
        } else
            continue;
        q.delta = delta;
        fwrite(&q, sizeof(q), 1, fp);
    }
    fclose(fp);
    fclose(input);
//CHECK IF ALL ITEMS HAVE THE SAME FREQUENT
#ifdef UNIFORM_OPERATE
    if (isload) {
        delete tmp;
        return 1;
    }
    request * requests = requests_init(check_filename);
    size_t counter = 0;
    for (size_t i = 0; i < num_records; i++) {
        enum request_types type = (enum request_types)requests[i].type;
        char *key = requests[i].hashed_key;
        uint64_t value;

        int ret;
        if (type == request_get) {
            ret = memc_inc(memc, key, 0, &value);
            if (value == frequent) {
                counter++;
            } else if (value > frequent) {
                printf("item %ld's frequent is bigger then expeted! WRONG\n", i);
                break;
            }
        } else {
            fprintf(stderr, "unknown request type");
        }
    }

    FILE *isOK = fopen("uniform_check", "w");
    sprintf(tmp,"%ld",counter);
    if (counter == num_records) {
        fwrite("true\n", sizeof(char), 5, isOK);
        fwrite(tmp, sizeof(char), strlen(tmp), isOK);
    } else {
        fwrite("false\n", sizeof(char), 6, isOK);
        fwrite(tmp, sizeof(char), strlen(tmp), isOK);
    }
    fclose(isOK);
#endif
    delete tmp;
}
