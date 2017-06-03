#define _GNU_SOURCE
#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sched.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


#include <libmemcached/memcached.h>

#include "bench_common.h"

pthread_mutex_t printmutex;

typedef struct {
  size_t tid;
  request *requests;
  size_t num_ops;
  size_t num_puts;
  size_t num_gets;
  size_t num_miss;
  size_t num_hits;
  double tput;
  double time;
} thread_param;

/* default parameter settings */
static size_t key_len;
static size_t val_len;
static size_t num_requests;
static size_t num_threads = 1;
static size_t num_mget = 1;
static float duration = 10.0;
static char* serverip = NULL;
static char* inputfile = NULL;
static char* outputfolder = "./real_stream";
static char* workingset = NULL;
static bool isreverse = false;

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

  pthread_mutex_lock (&printmutex);
  sprintf(config_string, "--SERVER=%s --BINARY-PROTOCOL", serverip);
  printf("config_string = %s\n", config_string);
  memc = memcached(config_string, strlen(config_string));

  getter = memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_NO_BLOCK);
  printf("No block: %lld\n", getter);
  getter = memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE);
  printf("Socket send size: %lld\n", getter);
  getter = memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE);
  printf("Socket recv size: %lld\n", getter);

  pthread_mutex_unlock (&printmutex);
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

/* executing requests at each thread */
static void* requests_exec(void *param)
{
  /* create a memcached structure */
  memcached_st *memc;
  memc = memc_new();

  size_t unicItems = 0;
  char queryfile[1024];
  snprintf(queryfile, sizeof(queryfile), "%s/%s_query", outputfolder, workingset);
  
  FILE *fquery = fopen(queryfile, "w+");
  if (fquery == NULL){
      printf("Can't open query file: %s", queryfile);
      exit(-1);
  }
  fwrite(&key_len, sizeof(size_t), 1, fquery);
    fwrite(&val_len, sizeof(size_t), 1, fquery);
    fwrite(&unicItems, sizeof(size_t), 1, fquery);

//  char outputfile[1024];
//  snprintf(outputfile, sizeof(outputfile), "./benchmarks/%s.bench", workingset);

  /*
  FILE *fout = fopen(outputfile, "w");
  fprintf(fout, "key_len : %ld\n", key_len);
  fprintf(fout, "num_requests : %ld\n", num_requests);
  */
  struct timeval tv_s, tv_e;

  thread_param* p = (thread_param*) param;

  pthread_mutex_lock (&printmutex);
  printf("start benching using thread%"PRIu64"\n", p->tid);
  pthread_mutex_unlock (&printmutex);

  request* requests = p->requests;
  p->time = 0;
//  while (p->time < duration) {
//    gettimeofday(&tv_s, NULL);  // start timing

  request q;
  q.type = request_get;
  q.delta = 1;
      for (size_t i = 0 ; i < p->num_ops; i++) {
          enum request_types type = requests[i].type;
          char *key = requests[i].hashed_key;
          uint8_t delta = requests[i].delta;
          uint64_t value;

          int ret;

          /*
          if (type == request_init) {
              ret = memc_init_inc(memc, key, 0, 0, &value);
              if (ret == 0) p->num_puts++;
          } else
          */
          if (type == request_inc) {
              ret = memc_init_inc(memc, key, delta, delta, &value);
              if (ret == 0) {
                  p->num_gets++;
//                  printf("A:%ld", value);
                  if (value == delta){
                      memcpy(q.hashed_key, key, key_len);
                      fwrite(&q, sizeof(q), 1, fquery);
                      unicItems++;
//                      printf("B:%d", delta);
                  }
              }
          }
          /*
          else if (type == request_dec) {
              ret = memc_dec(memc, key, delta, &value);
              if (ret == 0) p->num_gets++;
          } else if (type == request_get) {
              ret = memc_inc(memc, key, 0, &value);
              //fwrite(key, NKEY, 1, fout);
              if (ret == 1) {
                  printf("X");
                  //printf("%s", memcached_strerror(memc, ret));
                  fprintf(fout, "%ld\n", (uint64_t)0);
              } else {
                  p->num_gets++;
                  fprintf(fout, "%ld\n", value);
              }
          }
          */
          else {
              fprintf(stderr, "unknown request type");
          }
      }
//      fclose(fquery);

      //     FILE *fquery2 = fopen(queryfile, "w+");
      fseek(fquery, 2*sizeof(size_t), SEEK_SET);
      fwrite(&unicItems, sizeof(size_t), 1, fquery);
      fclose(fquery);
      
  size_t nops = p->num_gets + p->num_puts;
  p->tput = nops / p->time;

  pthread_mutex_lock (&printmutex);
  printf("thread%"PRIu64" gets %"PRIu64" items in %.2f sec \n",
         p->tid, nops, p->time);
  printf("#put = %zu, #get = %zu\n", p->num_puts, p->num_gets);
  printf("#miss = %zu, #hits = %zu\n", p->num_miss, p->num_hits);
  printf("hitratio = %.4f\n",   (float) p->num_hits / p->num_gets);
  printf("tput = %.2f\n",  p->tput);
  printf("\n");
  pthread_mutex_unlock (&printmutex);

  memcached_free(memc);

  printf("requests_exec...done\n");
  pthread_exit(NULL);
}

static void usage(char* binname)
{
  printf("%s [-t #] [-b #] [-l trace] [-d #] [-h] [-o outputfolder] [-s IP] [-r]\n", binname);
  printf("\t-t #: number of working threads, by default %"PRIu64"\n", num_threads);
  //printf("\t-b #: batch size for mget, by default %"PRIu64"\n", num_mget);
  printf("\t-d #: duration of the test in seconds, by default %f\n", duration);
  printf("\t-s IP: memcached server, by default 127.0.0.1 (localhost), required\n");
  printf("\t-l trace: e.g., /path/to/ycsbtrace, required\n");
  printf("\t-o outputfolder: default ./real_stream");
  printf("\t-w workingset: e.g., kv1M_op1M_uniform, required\n");
  printf("\t-r : reverse the process of operation\n");
  printf("\t-h  : show usage\n");
}


int
main(int argc, char **argv)
{
  if (argc <= 1) {
    usage(argv[0]);
    exit(-1);
  }

  char ch;
  while ((ch = getopt(argc, argv, "t:b:d:s:l:w:o:rh")) != -1) {
    switch (ch) {
//    case 't': num_threads = atoi(optarg); break;
    case 't': num_threads = 1; break;
    //case 'b': num_mget    = atoi(optarg); break;
    case 'd': duration    = atof(optarg); break;
    case 's': serverip    = optarg; break;
    case 'l': inputfile   = optarg; break;
    case 'o': outputfolder   = optarg; break;
    case 'w': workingset  = optarg; break;
    case 'r': isreverse   = true; break;
    case 'h': usage(argv[0]); exit(0); break;
    default:
      usage(argv[0]);
      exit(-1);
    }
  }

  if (serverip == NULL || inputfile == NULL || workingset == NULL) {
    usage(argv[0]);
    exit(-1);
  }

  request *requests = requests_init(inputfile);

  pthread_t threads[num_threads];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);;

  pthread_mutex_init(&printmutex, NULL);

  size_t t;
  thread_param tp[num_threads];
  for (t = 0; t < num_threads; t++) {
    tp[t].requests = requests + t * (num_requests / num_threads);
    tp[t].tid     = t;
    tp[t].num_ops = num_requests / num_threads;
    tp[t].num_puts = tp[t].num_gets = tp[t].num_miss = tp[t].num_hits = 0;
    tp[t].time = tp[t].tput = 0.0;
    int rc = pthread_create(&threads[t], &attr, requests_exec, (void *) &tp[t]);
    if (rc) {
      perror("failed: pthread_create\n");
      exit(-1);
    }
  }

  result_t result;
  result.total_time = 0.0;;
  result.total_tput = 0.0;
  result.total_hits = 0;
  result.total_miss = 0;
  result.total_gets = 0;
  result.total_puts = 0;
  result.num_threads = num_threads;

  for (t = 0; t < num_threads; t++) {
    void *status;
    int rc = pthread_join(threads[t], &status);
    if (rc) {
      perror("error, pthread_join\n");
      exit(-1);
    }
    result.total_time = (result.total_time > tp[t].time) ? result.total_time : tp[t].time;
    result.total_tput += tp[t].tput;
    result.total_hits += tp[t].num_hits;
    result.total_miss += tp[t].num_miss;
    result.total_gets += tp[t].num_gets;
    result.total_puts += tp[t].num_puts;
  }

  printf("total_time = %.2f\n", result.total_time);
  printf("total_tput = %.2f\n", result.total_tput);
  printf("total_hitratio = %.4f\n", (float) result.total_hits / result.total_gets);

  pthread_attr_destroy(&attr);
  printf("bye\n");
  return 0;
}
