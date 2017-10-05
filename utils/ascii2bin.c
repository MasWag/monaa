#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  int result;
  FILE *ifile = stdin;
  FILE *ofile = stdout;
  int N;
  char event;
  double timestamp;
  int i = 0;
  
  while((result = getopt(argc,argv,"i:o:")) != -1) {
    switch(result){
    case 'i':
      if (!(ifile = fopen(optarg, "r"))) {
        perror("ascii2bin:");
        return 1;
      }
      break;
    case 'o':
      if (!(ofile = fopen(optarg, "w+"))) {
        perror("ascii2bin");
        return 1;
      }
      break;
    }
  }

  fscanf(ifile, " %d\n", &N);
  fwrite(&N, sizeof(int), 1, ofile);
  for (i = 0; i < N; i++) {
    fscanf(ifile, " %c %lf\n", &event, &timestamp);
    fwrite(&event, sizeof(char), 1, ofile);
    fwrite(&timestamp, sizeof(double), 1, ofile);
  }

  return 0;
}

