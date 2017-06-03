#include <getopt.h>
#include "bench_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
static bool isreverse;
/* default parameter settings */

size_t c=0,d=0, s=0;
bool type_flag = false;
enum request_types type = request_dec;
static size_t key_len;
static size_t val_len;
static size_t num_requests;
//static char finit_up[] = "init_up";

static void cut(char* filename)
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
  if (s > num_requests) {
      perror("Start position is out of range");
      exit(-1);
  }

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

  FILE *fout = NULL;
  char foutput[100] = "cut.out";
  size_t index = 0;
//-----------for base------------
    sprintf(foutput,"cut_base.out");
    fout = fopen(foutput, "w");
    fwrite(&key_len, sizeof(size_t), 1, fout);
    fwrite(&val_len, sizeof(size_t), 1, fout);
    fwrite(&s, sizeof(size_t), 1, fout);
    for (int j=0; j<s; j++) {
        fwrite(&requests[index], sizeof(request), 1, fout);
        index++;
    }

    fclose(fout);
//--------for cut--------
  for (int i = 0; i < c; i++) {
    sprintf(foutput,"cut_%d.out",i);
    fout = fopen(foutput, "w");
    fwrite(&key_len, sizeof(size_t), 1, fout);
    fwrite(&val_len, sizeof(size_t), 1, fout);
//    fwrite(&requests[index], sizeof(request), d, fout);
    if (num_read - index < d) {
        d = num_read - index;
    }
    fwrite(&d, sizeof(size_t), 1, fout);
    for (int j=0; j<d; j++) {
        if(type_flag){
            requests[index].type = type;
        }

        fwrite(&requests[index], sizeof(request), 1, fout);
        index++;
    }

    fclose(fout);
    if (index >= num_read) {
        break;
    }
  }

  printf("cut...done\n");
}

static void usage(char* binname)
{//"usage: ./bench_update_gen  output_filename input_filename check_filename append_or_not[yes/no]"
  printf("%s [-f file] [-s start_position] [-c #] [-d #] [-t request_type] [-h]\n", binname);
  printf("\t-f trace: e.g., /path/to/cutFile, required\n");
  printf("\t-s start_position: at which position we cut, default 0\n");
  printf("\t-c # : number of chips to be cut, required\n");
  printf("\t-d # : number of record in each chip, required\n");
  printf("\t-t requset_type : default %d (decrease)\n", (int)request_dec);
  printf("\t\t %d (init)\n", (int)request_init);
  printf("\t\t %d (increase)\n", (int)request_inc);
  printf("\t\t %d (decrease)\n", (int)request_dec);
  printf("\t\t %d (query)\n", (int)request_get);
  printf("\t-h : show usage\n");
}





int main(int argc, char **argv)
{
  char* inputfile = NULL;
  if (argc <= 2) {
    usage(argv[0]);
    exit(-1);
  }
  char ch;
  type_flag = false;
  s = 0;
  while ((ch = getopt(argc, argv,
          "f:"
          "s:"
          "c:"
          "d:"
          "t:"
          "h"
        )) != -1) {
    switch (ch) {
    case 'f': inputfile = optarg; break;
    case 's': s = atoi(optarg); break;
    case 'c': c = atoi(optarg); break;
    case 'd': d = atoi(optarg); break;
    case 't': type = (enum request_types)atoi(optarg);
        type_flag = true;
        break;
    case 'h': usage(argv[0]); exit(0); break;
    default:
      usage(argv[0]);
      exit(-1);
    }
  }

  if (d==0 || inputfile == NULL) {
    usage(argv[0]);
    exit(-1);
  }
  cut(inputfile);

  return 0;
}
