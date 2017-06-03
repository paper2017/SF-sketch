#define _GNU_SOURCE
#include <getopt.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


#include <time.h>
#include <sys/time.h>

#include <libmemcached/memcached.h>

#include "bench_common.h"

typedef struct workload_t {
    size_t key_len;
    size_t val_len;
    size_t num;
    request *requests;
}workload;

/* default parameter settings */
static size_t key_len;
static size_t val_len;
static workload loads;
static workload querys;
static char* serverip = NULL;
static char* loadfile = NULL;
static char* queryfile = NULL;
static char* workingset = NULL;
static size_t interval = 0;
static memcached_st *memc;
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


/* create a memcached structure */
static memcached_st *memc_new()
{
  char config_string[1024];
  memcached_st *memc = NULL;
  unsigned long long getter;

  sprintf(config_string, "--SERVER=%s --BINARY-PROTOCOL", serverip);
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

/* init all requests from the ycsb trace file before issuing them */
static bool requests_init(char* filename, workload* pload)
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

  struct timeval tv_s, tv_e;
  double time;

  if (!test_throughput) {
      for (int i=0; i<querys.num; i++){
        enum request_types type = querys.requests[i].type;
        char *key = querys.requests[i].hashed_key;
        uint64_t value;

        int ret = 0;

        if (type == request_get) {
            ret = memc_inc(memc, key, 0, &value);
            if (ret == 1) {
                fprintf(fout, "%ld\n", (uint64_t)0);
            } else {
                fprintf(fout, "%ld\n", value);
            }
        } else {
            fprintf(stderr, "unknown request type");
        }
      }
  } else {
      FILE *ftime = fopen("throughput.bench", "w");

      gettimeofday(&tv_s, NULL);  // start timing
      for (int i=0; i<querys.num; i++){
        enum request_types type = querys.requests[i].type;
        char *key = querys.requests[i].hashed_key;
        uint64_t value;

        int ret = 0;

        if (type == request_get) {
            ret = memc_inc(memc, key, 0, &value);
        } else {
            fprintf(stderr, "unknown request type");
        }
      }

      gettimeofday(&tv_e, NULL);  // stop timing
      time = timeval_diff(&tv_s, &tv_e);
      fprintf(ftime, "query num : %ld items\n", querys.num);
      fprintf(ftime, "query time : %.2f sec\n", time);
      fprintf(ftime, "query throughput : %.2f thousand items/sec\n", querys.num / (time * 1000) );

      fclose(ftime);
  }



  fclose(fout);
}

static void requests_exec()
{
  /* create a memcached structure */
  memc = memc_new();

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
        char *key = loads.requests[index].hashed_key;
        uint8_t delta = loads.requests[index].delta;
        uint64_t value;

        int ret;

        if (type == request_inc) {
          ret = memc_inc(memc, key, delta, &value);
          //if (ret==1) printf("F");
        } else if (type == request_dec) {
          ret = memc_dec(memc, key, delta, &value);
        } else {
          fprintf(stderr, "unknown request type");
        }
        index++;
    }
    snprintf(outputfile, sizeof(outputfile), "./benchmarks/%s_%d.bench", workingset, count);
    queryfqs(outputfile);
  }

  index = loads.num - 1;
  if (!test_throughput) {
      while (index >= 0) {
          count--;
        for (int i=0; i < ops; i++) {
            enum request_types type = loads.requests[index].type;
            char *key = loads.requests[index].hashed_key;
            uint8_t delta = loads.requests[index].delta;
            uint64_t value;

            int ret;

            if (type == request_dec) {
              ret = memc_inc(memc, key, delta, &value);
            } else if (type == request_inc) {
              ret = memc_dec(memc, key, delta, &value);
            } else {
              fprintf(stderr, "unknown request type");
            }
            index--;
        }
        snprintf(outputfile, sizeof(outputfile), "./benchmarks/%s_r_%d.bench", workingset, count);
        queryfqs(outputfile);

        ops = interval;
      }
  }

  memcached_free(memc);

  printf("requests_exec...done\n");
}

static void usage(char* binname)
{
  printf("%s [-s IP] [-w workingset] [-l loadfile] [-q queryfile] [-r #] [t] [-h]\n", binname);
  printf("\t-s IP: memcached server, by default 127.0.0.1 (localhost), required\n");
  printf("\t-w workingset: e.g., kv1M_op1M_uniform, required\n");
  printf("\t-l loadfile, required\n");
  printf("\t-q queryfile, required\n");
  printf("\t-r : number of operations per interval, required\n");
  printf("\t-t : for test throughput only, required\n");
  printf("\t-h  : show usage\n");
}


int
main(int argc, char **argv)
{
  if (argc <= 1) {
    usage(argv[0]);
    exit(-1);
  }
  test_throughput = false;
  char ch;
  while ((ch = getopt(argc, argv, "s:w:l:q:r:th")) != -1) {
    switch (ch) {
    case 's': serverip    = optarg; break;
    case 'w': workingset  = optarg; break;
    case 'l': loadfile   = optarg; break;
    case 'q': queryfile   = optarg; break;
    case 'r': interval   = atoi(optarg); break;
    case 't': test_throughput   = true; break;
    case 'h': usage(argv[0]); exit(0); break;
    default:
      usage(argv[0]);
      exit(-1);
    }
  }

  if (serverip == NULL || loadfile == NULL || queryfile == NULL || workingset == NULL || interval == 0) {
    usage(argv[0]);
    exit(-1);
  }

  requests_init(loadfile, &loads);
  requests_init(queryfile, &querys);
  key_len = loads.key_len;
  val_len = loads.val_len;
  requests_exec();

  free(loads.requests);
  free(querys.requests);
  printf("bye\n");
  return 0;
}
