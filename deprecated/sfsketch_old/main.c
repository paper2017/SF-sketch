#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "sketch.h"
#define NKEY 20
#define NVAL 32
#ifdef SF1
#define QUERY_RESULT "sf1sketch_%d.out"
#endif
#ifdef SF2
#define QUERY_RESULT "sf2sketch_%d.out"
#endif
#ifdef SF3
#define QUERY_RESULT "sf3sketch_%d.out"
#endif
#ifdef SF4
#define QUERY_RESULT "sf4sketch_%d.out"
#endif
#ifdef SFF
#define QUERY_RESULT "sffsketch_%d.out"
#endif
/* type of each request */
enum request_types {
    INSERT_T=1,
    DELETE_T,
    QUERY_T
};

/*
 * format of each request, it has a key and a type and we don't care
 * the value
 */
typedef struct __attribute__((__packed__))
{
    char hashed_key[NKEY];
    char type;
    uint8_t delta;
}
request;

typedef struct workload_t {
    size_t key_len;
    size_t val_len;
    size_t num;
    request *requests;
} workload;

static size_t key_len;
static size_t val_len;
static size_t interval = 0;
static workload operations;
static workload queries;


static bool workload_init(char* filename, workload* p_operations)
{
    FILE *input;

    input = fopen(filename, "rb");
    if (input == NULL) {
        perror("Can not open file");
        perror(filename);
        exit(1);
    }

    p_operations->key_len = 0;
    p_operations->val_len = 0;
    p_operations->num = 0;
    p_operations->requests = NULL;

    int n;
    n = fread(&p_operations->key_len, 8, 1, input);
    if (n != 1)
        perror("fread error");
    n = fread(&p_operations->val_len, 8, 1, input);
    if (n != 1)
        perror("fread error");
    n = fread(&p_operations->num, 8, 1, input);
    if (n != 1)
        perror("fread error");

    printf("trace(%s):\n", filename);
    printf("\tkey_len = %zu\n", p_operations->key_len);
    printf("\tval_len = %zu\n", p_operations->val_len);
    printf("\tnum_requests = %zu\n", p_operations->num);
    printf("\n");

    p_operations->requests = malloc(sizeof(request) * p_operations->num);
    if (p_operations->requests == NULL) {
        perror("not enough memory to init requests\n");
        exit(-1);
    }

    uint64_t num_read;
    num_read = fread(p_operations->requests, sizeof(request), p_operations->num, input);
    if (num_read < p_operations->num) {
        fprintf(stderr, "num_read: %zu\n", num_read);
        perror("can not read all requests\n");
        fclose(input);
        exit(-1);
    }

    fclose(input);
    printf("workload_init...done\n");
    return true;
}

static void query_freq(char* fn_output)
{
    FILE *fout = fopen(fn_output, "w");
    fprintf(fout, "key_len : %ld\n", queries.key_len);
    fprintf(fout, "num_requests : %ld\n", queries.num);

    for (int i=0; i<queries.num; i++) {
        enum request_types type = queries.requests[i].type;
        const unsigned char *key = (unsigned char *)queries.requests[i].hashed_key;
        uint64_t value;

        if (type == QUERY_T) {
            value = query(key, key_len);
            fprintf(fout, "%ld\n", value);
        }
    }

    fclose(fout);
}

static void requests_exec()
{
    char fn_output[1024];
    int ops = interval;
    long int index = 0;
    int count = 0;
    while (index < operations.num) {
        if (operations.num - index < interval) {
            ops = operations.num - index;
        }
        count++;
        for (int i=0; i < ops; i++) {
            enum request_types type = operations.requests[index].type;
            const unsigned char *key = (unsigned char *)operations.requests[index].hashed_key;
            uint8_t delta = operations.requests[index].delta;

            if (type == INSERT_T) {
                inc(key, key_len, delta);
            }
#if (SF2 + SF3 + SF4 + SFF == 1)
            else if (type == DELETE_T) {
                dec(key, key_len, delta);
            }
#endif
            index++;
        }
        snprintf(fn_output, sizeof(fn_output), QUERY_RESULT, count);
        query_freq(fn_output);
    }
    printf("requests_exec...done\n");
}
static void usage(char* binname)
{
    //"usage: ./bench_update_gen  output_filename input_filename check_filename append_or_not[yes/no]"
    printf("%s [-o operation_file_name] [-q query_file_name] [-d #] [-w #] [-z #] [-b #] [-i #] [-h]\n", binname);
    printf("\t-o filename : file name for operations, default as operations.dat\n");
    printf("\t-q filename : file name for queries, default as queries.dat\n");
    printf("\t-d # : number of array in the Sketch, default as 5\n");
    printf("\t-w # : number of buckets in an array, default as 40000\n");
    printf("\t-z # : number of slots in a bucket for Fat sketch/Multi-Counter sketch,"\
           " default as 3\n");
    printf("\t-b # : number of bits used to store a counter, must be less than %ld (default)\n",
           8*sizeof(size_t));
    printf("\t-i # : number of operations per interval"\
           " (default as the total operations)\n");
    printf("\t-h : show usage\n");
}





int main(int argc, char **argv)
{
    //"fn" short for "filename"
    char *fn_operations;
    fn_operations = "operations.dat";
    char *fn_queries;
    fn_queries = "queries.dat";
    interval = 0;
    size_t D=5,W=40000,Z=3,Bits_c=8*sizeof(size_t);
    char ch;
    while ((ch = getopt(argc, argv,
                        "o:"
                        "q:"
                        "d:"
                        "w:"
                        "z:"
                        "b:"
                        "i:"
                        "h"
                       )) != -1) {
        switch (ch) {
        case 'o':
            fn_operations = optarg;
            break;
        case 'q':
            fn_queries = optarg;
            break;
        case 'd':
            D = atoi(optarg);
            break;
        case 'w':
            W = atoi(optarg);
            break;
        case 'z':
            Z = atoi(optarg);
            break;
        case 'b':
            Bits_c = atoi(optarg);
            break;
        case 'i':
            interval   = atoi(optarg);
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

    if (D==0 || W==0 || Z==0 || Bits_c==0) {
        usage(argv[0]);
        exit(-1);
    }

    if ((access(fn_operations, 0)) == -1) {
        printf("Operation File: %s doesn't exist!\n", fn_operations);
        usage(argv[0]);
        exit(-1);
    }
    if ((access(fn_queries, 0)) == -1) {
        printf("Query File: %s doesn't exist!\n", fn_queries);
        usage(argv[0]);
        exit(-1);
    }

    init(D, W, Z, Bits_c);

    workload_init(fn_queries, &queries);
    workload_init(fn_operations, &operations);
    key_len = operations.key_len;
    val_len = operations.val_len;
    if (key_len != queries.key_len || val_len != queries.val_len) {
        printf("Error operations/queries files\n");
        return 1;
    }
    if (interval == 0) {
        interval = operations.num;
    }
    printf("interval: %ld; opt_num:%ld\n", interval, operations.num);
    requests_exec();

    printf("bye\n");
    return 0;
}
