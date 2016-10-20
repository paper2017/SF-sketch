#include <time.h>
#include <sys/time.h>

#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "sketch.h"
#include "../bench_sketch/bench_common.h"

#include <errno.h>

int __cdecl gettimeofday(struct timeval *__restrict__,
                         void *__restrict__  /* tzp (unused) */);

typedef struct workload_t {
    size_t key_len;
    size_t val_len;
    size_t num;
    request *requests;
}workload;

static bool isreverse;
/* default parameter settings */
static size_t key_len;
static size_t val_len;
static size_t num_requests;
static size_t interval = 0;
static workload loads;
static workload querys;

static bool test_throughput = false;
/* Calculate the second difference*/
static double timeval_diff(struct timeval *start,
                           struct timeval *end)
{
  double r = end->tv_sec - start->tv_sec;

  /* Calculate the microsecond difference */
  if (end->tv_usec > start->tv_usec)
    r += (end->tv_usec - start->tv_usec)/1000000.0;
  else if (end->tv_usec < start->tv_usec)
    r -= (start->tv_usec - end->tv_usec)/1000000.0;
  return r;
}
//static char finit_up[] = "init_up";

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
  n = fread(&key_len, sizeof(key_len), 1, input);
  if (n != 1)
    perror("fread error");
  n = fread(&val_len, sizeof(val_len), 1, input);
  if (n != 1)
    perror("fread error");
  n = fread(&num_requests, sizeof(num_requests), 1, input);
  if (n != 1)
    perror("fread error");

  printf("trace(%s):\n", filename);
  printf("\tkey_len = %zu\n", key_len);
  printf("\tval_len = %zu\n", val_len);
  printf("\tnum_requests = %zu\n", num_requests);
  printf("\n");

  request *requests = malloc(sizeof(request) * num_requests);
  if (requests == NULL) {
    perror("not enough memory to init requests\n");
    exit(-1);
  }

  size_t num_read;
  num_read = fread(requests, sizeof(request), num_requests, input);
  if (num_read < num_requests) {
    fprintf(stderr, "num_read: %zu\n", num_read);
    perror("can not read all requests\n");
    fclose(input);
    exit(-1);
  }

  fclose(input);
  printf("requests_init...done\n");
  return requests;
}


static void test_thrpt(request *requests){
  struct timeval tv_s, tv_e;
  double time;
  FILE *ftime = fopen("throughput_fcsketch.out", "w");
  gettimeofday(&tv_s, NULL);  // start timing
  for (int i=0; i<num_requests; i++){
    enum request_types type = requests[i].type;
    const unsigned char *key = (unsigned char *)requests[i].hashed_key;

    if (type == request_get) {
        query(key, key_len);
    } else {
        fprintf(stderr, "unknown request type");
    }
  }
  gettimeofday(&tv_e, NULL);  // stop timing
  time = timeval_diff(&tv_s, &tv_e);
  fprintf(ftime, "query num : %ld items\n", num_requests);
  fprintf(ftime, "query time : %.2f sec\n", time);
  fprintf(ftime, "query throughput : %.2f thousand items/sec\n", num_requests / (time * 1000) );

  fclose(ftime);
}


/* executing requests at each thread */
static void requests_exec(request *requests, char* filename)
{
    FILE *fout = fopen(filename, "w");
    fprintf(fout, "key_len : %ld\n", key_len);
    fprintf(fout, "num_requests : %ld\n", num_requests);

    if (!isreverse) {
        for (size_t i = 0 ; i < num_requests; i++) {
            enum request_types type = requests[i].type;
            const unsigned char *key = (unsigned char *)requests[i].hashed_key;
            uint8_t delta = requests[i].delta;

            if (type == request_init) {
                inc(key, key_len, delta);
            } else if (type == request_inc) {
                inc(key, key_len, delta);
            }
#if (FC_V2 + FC_V3 + FC_V4 == 1)
            else if (type == request_dec) {
                dec(key, key_len, delta);
            }
#endif
            else if (type == request_get) {
                size_t val = query(key, key_len);
                fprintf(fout, "%ld\n", val);
            } else {
                fprintf(stderr, "unknown request type\n");
            }
        }
    } else {
        for (size_t i = num_requests; i >= 1;) {
            i--;
            enum request_types type = requests[i].type;
            const unsigned char *key = (unsigned char *)requests[i].hashed_key;
            uint8_t delta = requests[i].delta;
#if (FC_V2 + FC_V3 + FC_V4 == 1)
            if (type == request_inc) {
                dec(key, key_len, delta);
            } else if (type == request_dec) {
                inc(key, key_len, delta);
            } else {
                fprintf(stderr, "unknown request type\n");
            }
#endif
        }
    }
    fclose(fout);

  printf("requests_exec...done\n");
}



static bool requests_init_r(char* filename, workload* pload)
{
  FILE *input;

  input = fopen(filename, "rb");
  if (input == NULL) {
    perror("can not open file");
    perror(filename);
    exit(1);
  }

  pload->key_len = 0;
  pload->val_len = 0;
  pload->num = 0;
  pload->requests = NULL;

  int n;
  n = fread(&pload->key_len, sizeof(pload->key_len), 1, input);
  if (n != 1)
    perror("fread error");
  n = fread(&pload->val_len, sizeof(pload->val_len), 1, input);
  if (n != 1)
    perror("fread error");
  n = fread(&pload->num, sizeof(pload->num), 1, input);
  if (n != 1)
    perror("fread error");

  printf("trace(%s):\n", filename);
  printf("\tkey_len = %zu\n", pload->key_len);
  printf("\tval_len = %zu\n", pload->val_len);
  printf("\tnum_requests = %zu\n", pload->num);
  printf("\n");

  pload->requests = malloc(sizeof(request) * pload->num);
  if (pload->requests == NULL) {
    perror("not enough memory to init requests\n");
    exit(-1);
  }

  size_t num_read;
  num_read = fread(pload->requests, sizeof(request), pload->num, input);
  if (num_read < pload->num) {
    fprintf(stderr, "num_read: %zu\n", num_read);
    perror("can not read all requests\n");
    fclose(input);
    exit(-1);
  }

  fclose(input);
  printf("requests_init...done\n");
  return true;
}

static void queryfqs(char* outputfile){
  FILE *fout = fopen(outputfile, "w");
  fprintf(fout, "key_len : %ld\n", querys.key_len);
  fprintf(fout, "num_requests : %ld\n", querys.num);

  for (int i=0; i<querys.num; i++){
    enum request_types type = querys.requests[i].type;
    const unsigned char *key = (unsigned char *)querys.requests[i].hashed_key;
    size_t value;

    if (type == request_get) {
        value = query(key, key_len);
        fprintf(fout, "%ld\n", value);
    } else {
        fprintf(stderr, "unknown request type");
    }
  }

  fclose(fout);
}

static void requests_exec_r()
{
  char outputfile[1024];
  int ops = interval;
  long int index = 0;
  int count = 0;
  while (index < loads.num) {
    if (loads.num - index < interval) {
        ops = loads.num - index;
    }
    count++;
    for (int i=0; i < ops; i++) {
        enum request_types type = loads.requests[index].type;
        const unsigned char *key = (unsigned char *)loads.requests[index].hashed_key;
        uint8_t delta = loads.requests[index].delta;

        if (type == request_inc) {
          inc(key, key_len, delta);
        } else if (type == request_dec) {
          dec(key, key_len, delta);
        } else {
          fprintf(stderr, "unknown request type");
        }
        index++;
    }
    snprintf(outputfile, sizeof(outputfile), "fcsketch_%d.out", count);
    queryfqs(outputfile);
  }
  index = loads.num - 1;
  while (index >= 0) {
      count--;
    for (int i=0; i < ops; i++) {
        enum request_types type = loads.requests[index].type;
        const unsigned char *key = (unsigned char *)loads.requests[index].hashed_key;
        uint8_t delta = loads.requests[index].delta;
        if (type == request_dec) {
          inc(key, key_len, delta);
        } else if (type == request_inc) {
          dec(key, key_len, delta);
        } else {
          fprintf(stderr, "unknown request type");
        }
        index--;
    }
    snprintf(outputfile, sizeof(outputfile), "fcsketch_r_%d.out", count);
    queryfqs(outputfile);
    ops = interval;
  }
  printf("requests_exec...done\n");
}
static void usage(char* binname)
{//"usage: ./bench_update_gen  output_filename input_filename check_filename append_or_not[yes/no]"
  printf("%s [-d #] [-w #] [-k #] [-b #] [-n #] [-r #] [t] [-h]\n", binname);
  printf("\t-d # : number of tables on/off chip in a fcsketch, required\n");
  printf("\t-w # : number of buckets in a table, required\n");
  printf("\t-k # : number of slots in a off-chip bucket, required\n");
  printf("\t-b # : number of bits used to store a counter, must be less than %ld\n", 8*sizeof(size_t));
  printf("\t-n # : number of subfiles used in this experiment\n");
  printf("\t-r # : number of operations per interval\n");
  printf("\t-t : for test throughput only.\n");
  printf("\t-h : show usage\n");
}





int main(int argc, char **argv)
{
  if (argc <= 2) {
    usage(argv[0]);
    exit(-1);
  }
  size_t D=0,W=0,K=0,Bits_c=8*sizeof(size_t);
  int nSubfiles=0;
  isreverse = false;
  bool curReverse = false;
  test_throughput = false;
  char ch;
  while ((ch = getopt(argc, argv,
          "d:"
          "w:"
          "k:"
          "b:"
          "n:"
          "r:"
          "t"
          "h"
        )) != -1) {
    switch (ch) {
    case 'd': D = atoi(optarg); break;
    case 'w': W = atoi(optarg); break;
    case 'k': K = atoi(optarg); break;
    case 'b': Bits_c = atoi(optarg); break;
    case 'n': nSubfiles = atoi(optarg); break;
    case 'r': curReverse   = true;interval   = atoi(optarg); break;
    case 't': test_throughput   = true; break;
    case 'h': usage(argv[0]); exit(0); break;
    default:
      usage(argv[0]);
      exit(-1);
    }
  }

  if (D==0 || W==0 || K==0 || Bits_c==0 || nSubfiles<0) {
    usage(argv[0]);
    exit(-1);
  }

  if ((curReverse == true) && (nSubfiles > 0)) {
    usage(argv[0]);
    exit(-1);
  }
//  init(50, 4000000, 1.5, 6);
//    init(50, 4000000, 1.5, 24);
//  init(5, 40000, 3, 24);
  init(D, W, K, Bits_c);
  request *requests = NULL;
/*
  requests = requests_init(finit_up);
  requests_exec(requests);
  free(requests);
  requests = NULL;
*/
  char finc_dec_up[100] = UPDATE_FILE;
  char fquery[] = QUERY_FILE;
  char foutput[100] = "fcsketch.out";

  if (curReverse){
    requests_init_r(finc_dec_up, &loads);
    requests_init_r(fquery, &querys);
    key_len = loads.key_len;
    val_len = loads.val_len;
    requests_exec_r();
  } else {
     requests = requests_init(finc_dec_up);
     requests_exec(requests, foutput);
     free(requests);
     requests = NULL;

     requests = requests_init(fquery);
     requests_exec(requests, foutput);

     if (test_throughput) {
        test_thrpt(requests);
     }
     free(requests);
     requests = NULL;

      for (int i=1; i<=nSubfiles; i++){
          sprintf(finc_dec_up,"inc-dec_up%d",i);
          sprintf(foutput,"fcsketch%d.out",i);
          requests = requests_init(finc_dec_up);
          requests_exec(requests, foutput);
          free(requests);
          requests = NULL;

          requests = requests_init(fquery);
          requests_exec(requests, foutput);
          free(requests);
          requests = NULL;
      }
  }

  printf("bye\n");
  return 0;
}
