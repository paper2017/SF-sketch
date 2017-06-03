#include <getopt.h>
#include "cmsketch.h"
#include "../bench_sketch/bench_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <cstring>
#include <string>

#include <time.h>
#include <sys/time.h>
using namespace std;

#ifdef CHECK_ABNORMAL
int vivi_counter = 0;      
#endif

typedef struct workload_t {
    size_t key_len;
    size_t val_len;
    size_t num;
    request *requests;
}workload;
static size_t interval = 0;
static workload loads;
static workload querys;

/* default parameter settings */
static size_t key_len;
static size_t val_len;
static size_t num_requests;
static bool isreverse;

static char EXP_RESULT_FILE_DEFAULT[128] = "timetest/exp_result_time";
static char * expRltFile = EXP_RESULT_FILE_DEFAULT;
static size_t n_startt = 0;
static timespec time_diff(timespec start, timespec end)
{
   timespec temp;
   if ((end.tv_nsec-start.tv_nsec)<0) {
      temp.tv_sec = end.tv_sec-start.tv_sec-1;
      temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
   } else {
      temp.tv_sec = end.tv_sec-start.tv_sec;
      temp.tv_nsec = end.tv_nsec-start.tv_nsec;
   }
   return temp;
}
static bool exp_time_test = false;
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

  request *requests = (request *)malloc(sizeof(request) * num_requests);
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

static void test_thrpt(request *requests, CountMinSketch & c){
  struct timeval tv_s, tv_e;
  double time;
  FILE *ftime = fopen("throughput_cmsketch.out", "w");
  gettimeofday(&tv_s, NULL);  // start timing
  for (int i=0; i<num_requests; i++){
    enum request_types type = (enum request_types)requests[i].type;
    const unsigned char *key = (unsigned char *)requests[i].hashed_key;

    if (type == request_get) {
        c.queryPoint(key, key_len);
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
static void requests_exec(request *requests, CountMinSketch & c, char *filename)
{
  FILE *fout = fopen(filename, "w");
  fprintf(fout, "key_len : %ld\n", key_len);
  fprintf(fout, "num_requests : %ld\n", num_requests);
  
    timespec delta_time_proc;
    //timespec delta_time_trd;
    timespec start_time_proc, end_time_proc;
    //timespec start_time_trd, end_time_trd;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time_proc);
    //clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time_trd);
    long int i = 1;
  if (!isreverse) {
    for (i = 0 ; i < num_requests; i++) {
        enum request_types type = (enum request_types)requests[i].type;
      const unsigned char *key = (unsigned char *)requests[i].hashed_key;
      uint8_t delta = requests[i].delta;

	  	if(unlikely(i == n_startt)) {
		printf("Exec Timming Start : %ld\n", i);
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time_proc);
			//clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time_trd);
		}
	  
      if (type == request_init) {
//        inc(key, key_len, delta);
          c.insert(key, key_len, delta);
      } else if (type == request_inc) {
//        inc(key, key_len, delta);
          c.insert(key, key_len, delta);
      }
#ifdef DECREASE_SUPPORT
       else if (type == request_dec) {
//        dec(key, key_len, delta);
           c.delete_(key, key_len, delta);
      }
#endif
       else if (type == request_get) {
       size_t val = c.queryPoint(key, key_len);
//        size_t val = query(key, key_len);
       //fwrite(key, NKEY, 1, fout);
	//if(unlikely(exp_time_test)){
			fprintf(fout, "%ld\n", val);
	//}
      } else {
        fprintf(stderr, "unknown request type\n");
      }
    }
  } else {
      for (i = num_requests; i >= 1;) {
	  	if(unlikely(i == n_startt)) {
		printf("Exec Timming Start : %ld\n", i);
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time_proc);
			//clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time_trd);
		}
          i--;
          enum request_types type = (enum request_types)requests[i].type;
          const unsigned char *key = (unsigned char *)requests[i].hashed_key;
          uint8_t delta = requests[i].delta;

#ifdef DECREASE_SUPPORT
          if (type == request_inc) {
              c.delete_(key, key_len, delta);
          } else if (type == request_dec) {
              c.insert(key, key_len, delta);
          } else {
              fprintf(stderr, "unknown request type\n");
          }
#endif
    }
  }
  
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time_proc);
    //clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time_trd);

	printf("Exec Timming End : %ld\n", i);
	
    delta_time_proc = time_diff(start_time_proc, end_time_proc);
    //delta_time_trd = diff(start_time_trd, end_time_trd);

    FILE * fExpRlt = fopen(expRltFile, "a");
    if (!fExpRlt) {
        fprintf(stderr, "File not found.\n");
        exit(0);
    }

    fprintf(fExpRlt, "%f ",
            delta_time_proc.tv_sec +
            (double)delta_time_proc.tv_nsec / 1000000000);
    fclose(fExpRlt);
  
  fclose(fout);

  printf("requests_exec...done\n");
}

static void requests_query(request *requests, CountMinSketch & c)
{
	long int i = 0;
    timespec delta_time_proc;
    //timespec delta_time_trd;
    timespec start_time_proc, end_time_proc;
    //timespec start_time_trd, end_time_trd;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time_proc);
	//clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time_trd);

		for (i = 0 ; i < n_startt; i++) {
            //enum request_types type = requests[i].type;
            const unsigned char *key = (unsigned char *)requests[i].hashed_key;
            c.queryPoint(key, key_len);
        }
		
		printf("Query Timming Start : %ld\n", i);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time_proc);
		//clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time_trd);

#ifdef CHECK_ABNORMAL
	vivi_counter = 0;
#endif
		
		for (; i < num_requests; i++) {
            //enum request_types type = requests[i].type;
            const unsigned char *key = (unsigned char *)requests[i].hashed_key;
            c.queryPoint(key, key_len);

        }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time_proc);
    //clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time_trd);

	printf("Query Timming End : %ld\n", i);
#ifdef CHECK_ABNORMAL
	printf("vivi_counter = %d \n", vivi_counter);
#endif	
    delta_time_proc = time_diff(start_time_proc, end_time_proc);
    //delta_time_trd = diff(start_time_trd, end_time_trd);

    FILE * fExpRlt = fopen(expRltFile, "a");
    if (!fExpRlt) {
        fprintf(stderr, "File not found.\n");
        exit(0);
    }

    fprintf(fExpRlt, "%f ",
            delta_time_proc.tv_sec +
            (double)delta_time_proc.tv_nsec / 1000000000);
#ifdef CHECK_ABNORMAL
	fprintf(fExpRlt, "%d ", vivi_counter);
#endif

			fclose(fExpRlt);

	printf("requests_query...done\n");
}

static void requests_delete(request *requests, CountMinSketch & c)
{
	long int i = 0;
    timespec delta_time_proc;
    //timespec delta_time_trd;
    timespec start_time_proc, end_time_proc;
    //timespec start_time_trd, end_time_trd;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time_proc);
	//clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time_trd);

		for (i = num_requests-1; i >= (num_requests - n_startt); i--) {
            //enum request_types type = requests[i].type;
            const unsigned char *key = (unsigned char *)requests[i].hashed_key;
            uint8_t delta = requests[i].delta;
			c.delete_(key, key_len, delta);
        }
		
		printf("Delete Timming Start : %ld\n", i);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time_proc);
		//clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time_trd);
		
		for (; i >= 0; i--) {
            //enum request_types type = requests[i].type;
            const unsigned char *key = (unsigned char *)requests[i].hashed_key;
            uint8_t delta = requests[i].delta;
			c.delete_(key, key_len, delta);
        }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time_proc);
    //clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time_trd);

	printf("Delete Timming End : %ld\n", i);
	
    delta_time_proc = time_diff(start_time_proc, end_time_proc);
    //delta_time_trd = diff(start_time_trd, end_time_trd);

    FILE * fExpRlt = fopen(expRltFile, "a");
    if (!fExpRlt) {
        fprintf(stderr, "File not found.\n");
        exit(0);
    }

    fprintf(fExpRlt, "%f ",
            delta_time_proc.tv_sec +
            (double)delta_time_proc.tv_nsec / 1000000000);
    fclose(fExpRlt);

	printf("requests_delete...done\n");
}

//--------------------
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

  pload->requests = (request *)malloc(sizeof(request) * pload->num);
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
static void queryfqs(char* outputfile, CountMinSketch & c){
  FILE *fout = fopen(outputfile, "w");
  fprintf(fout, "key_len : %ld\n", querys.key_len);
  fprintf(fout, "num_requests : %ld\n", querys.num);

  for (int i=0; i<querys.num; i++){
      enum request_types type = (enum request_types)querys.requests[i].type;
    const unsigned char *key = (unsigned char *)querys.requests[i].hashed_key;
    size_t value;

    if (type == request_get) {
        value = c.queryPoint(key, key_len);
        fprintf(fout, "%ld\n", value);
    } else {
        fprintf(stderr, "unknown request type");
    }
  }

  fclose(fout);
}


static void requests_exec_r(CountMinSketch & c)
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
        enum request_types type = (enum request_types)loads.requests[index].type;
        const unsigned char *key = (unsigned char *)loads.requests[index].hashed_key;
        uint8_t delta = loads.requests[index].delta;

        if (type == request_inc) {
          c.insert(key, key_len, delta);
        } else if (type == request_dec) {
          c.delete_(key, key_len, delta);
        } else {
          fprintf(stderr, "unknown request type");
        }
        index++;
    }
    snprintf(outputfile, sizeof(outputfile), "cmsketch_%d.out", count);
    queryfqs(outputfile, c);
  }
  index = loads.num - 1;
  while (index >= 0) {
      count--;
    for (int i=0; i < ops; i++) {
        enum request_types type = (enum request_types)loads.requests[index].type;
        const unsigned char *key = (unsigned char *)loads.requests[index].hashed_key;
        uint8_t delta = loads.requests[index].delta;
        if (type == request_dec) {
          c.insert(key, key_len, delta);
        } else if (type == request_inc) {
          c.delete_(key, key_len, delta);
        } else {
          fprintf(stderr, "unknown request type");
        }
        index--;
    }
    snprintf(outputfile, sizeof(outputfile), "cmsketch_r_%d.out", count);
    queryfqs(outputfile, c);
    ops = interval;
  }
  printf("requests_exec...done\n");
}

//--------------------


static void usage(char* binname)
{
  printf("%s [-d #] [-w #] [-b #] [-n #] [r #] [t] [-h]\n", binname);
  printf("\t-d # : number of tables on/off chip in a cmsketch, required\n");
  printf("\t-w # : number of buckets in a table, required\n");
  printf("\t-b # : number of bits used to store a counter, must be less than %ld\n", 8*sizeof(size_t));
  printf("\t-n # : number of subfiles used in this experiment\n");
  printf("\t-r # : number of operations per interval\n");
  printf("\t-t : for test throughput only.\n");
  printf("\t-h : show usage\n");
}

int main(int argc, char **argv)
{
  if (argc <= 1) {
    usage(argv[0]);
    exit(-1);
  }
  size_t D=0,W=0,Bits_c=8*sizeof(size_t);
  int nSubfiles=0;
  isreverse = false;
  bool curReverse = false;
  test_throughput = false;
  char ch;
  while ((ch = getopt(argc, argv,
          "d:"
          "w:"
          "b:"
          "n:"
          "r:"
          "t"
          "h"
          "e:"
	  "T"
	  "s:"
        )) != -1) {
    switch (ch) {
    case 'd': D = atoi(optarg); break;
    case 'w': W = atoi(optarg); break;
    case 'b': Bits_c = atoi(optarg); break;
    case 'n': nSubfiles = atoi(optarg); break;
    case 'r': curReverse   = true;interval   = atoi(optarg); break;
    case 't': test_throughput   = true; break;
    case 'h': usage(argv[0]); exit(0); break;
    case 'e': expRltFile = optarg; break;
    case 'T': exp_time_test = true; break;
    case 's': n_startt = atoi(optarg); break;
    default:
      usage(argv[0]);
      exit(-1);
    }
  }

  if (D==0 || W==0 || Bits_c==0 || nSubfiles<0) {
    usage(argv[0]);
    exit(-1);
  }
  if ((curReverse == true) && (nSubfiles > 0)) {
    usage(argv[0]);
    exit(-1);
  }

  CountMinSketch c(D, W, Bits_c);
  request *requests = NULL;

  char finc_dec_up[100] = "inc-dec_up";
  char fquery[] = "query";
  char foutput[100] = "cmsketch.out";

  if (curReverse){
    requests_init_r(finc_dec_up, &loads);
    requests_init_r(fquery, &querys);
    key_len = loads.key_len;
    val_len = loads.val_len;
    requests_exec_r(c);
  }else {
    requests = requests_init(finc_dec_up);
    requests_exec(requests, c, foutput);
	
	 if(exp_time_test) {
		 requests_query(requests, c);
		 requests_delete(requests, c);
	 }else{
		free(requests);
		requests = NULL;
		requests = requests_init(fquery);
		requests_exec(requests, c, foutput);
	 }

     if (test_throughput) {
        test_thrpt(requests, c);
     }
     free(requests);
     requests = NULL;

    for (int i=1; i<=nSubfiles; i++){
      sprintf(finc_dec_up,"inc-dec_up%d",i);
      sprintf(foutput,"cmsketch%d.out",i);
      requests = requests_init(finc_dec_up);
      requests_exec(requests, c, foutput);
      free(requests);
      requests = NULL;

      requests = requests_init(fquery);
      requests_exec(requests, c, foutput);
      free(requests);
      requests = NULL;
    }
  }

  printf("bye\n");
  return 0;
}
