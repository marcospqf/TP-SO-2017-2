#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<pwd.h>
#include<pthread.h>
#include<signal.h>
#include<time.h>
char pref[10];
char suf[10];

char *get_number(int num) {//transforma um numero numa cadeia de caracteres
  char *ret = malloc(11);
  int sz = 0;
  int aux = num;
  while(num > 0) {
    sz++;
    num /= 10;
  }
  num = aux;
  int i;
  for(i = sz - 1; i >= 0; i--) {
    char c = '0' + num % 10;
    ret[i] = c;
    num /= 10;
  }
  ret[sz] = '\0';
  return ret;
}

char *get_file_name(int pid) {//retorna o endereco absoluto do stat do processo de id = pid
  char *number = get_number(pid);
  char *ret = malloc((strlen(number) + 14) * sizeof(char));
  strcpy(ret, pref);
  strcpy(ret + 6, number);
  strcpy(ret + strlen(number) + 6, suf);
  return ret;
}

void print_spaces(int qt) {//imprime qt espacos para formatar
  int i;
  for(i = 0; i < qt; i++) printf(" ");
}

char *get_proc_name(FILE *file) {//retorna cadeia de caracteres correspondente ao nome do processo a ser tratado
  char *ret = malloc(100 * sizeof(char));
  char c;
  do {
    c = fgetc(file);
  } while(c == ' ' || c == '\0' || c == '\n');
  int pos = 0;
  do {
    ret[pos++] = c;
    c = fgetc(file);
  } while(c != ')');
  ret[pos] = '\0';
  return ret;
}

void to_print(FILE *file, char *usr) {//imprime um processo no top
  int pid, i;
  fscanf(file, "%d", &pid);
  char *num = get_number(pid);
  int sz = strlen(num);
  printf("%s", num);
  print_spaces(15 - sz);//Imprime o id do processo devidamente formatado
  printf("|");


  printf("%s", usr);//Imprime o owner do processo
  sz = strlen(usr);
  print_spaces(15 - sz);
  printf("|");


  char *proc_name = get_proc_name(file);
  sz = strlen(proc_name);
  char treated_proc[40];
  for(i = 1; i < sz - 1; i++) {
    treated_proc[i - 1] = proc_name[i];//Remove os '(' e ')' do nome do processo 
  }
  treated_proc[sz - 2] = '\0';
  printf("%s", treated_proc);//Imprime o nome do processo
  print_spaces(32 - sz);
  printf("|");


  char status[15];
  fscanf(file, "%s", status);
  printf("%s", status);//Imprime o status do processo
  sz = strlen(status);
  print_spaces(15 - sz);
  printf("|\n");
}

char *get_usr(char *file_name) {//retorna o owner do processo
  struct stat sb;
  stat(file_name, &sb);
  struct passwd *pw = getpwuid(sb.st_uid);
  return pw->pw_name;
}


int get_file(int pid) {
  char *file_name = get_file_name(pid);
  FILE *file = fopen(file_name, "r");
  if(!file) {//se nao existe o arquivo, o processo com esse id nao existe
    return 0;
  }
  char *usr = get_usr(file_name);
  to_print(file, usr);
  fclose(file);
  return 1;
}

void initialize() {//Imprime o cabecalho do top
  printf("PID            |User           |PROCNAME                      |Estado         |\n\n");
  printf("\n");
  printf("---------------|---------------|------------------------------|---------------|\n\n");
}

void run() {//Responsavel por imprimir 20 processos a cada segundo
  initialize();
  int n_process = 0;
  int process_id = rand() % 2500 + 1;
  while(n_process < 20) {
    process_id = rand() % 5000 + 1;
    if(!get_file(process_id)) continue;
    n_process++;
  }
}

void *thread_func_1(void *unused) {
  while(1) {
    printf("[3;J[H[2J");
    run();
    sleep(1);//tempo de espera para renovar o top
    printf("[3;J[H[2J");
  }
}

void *thread_func_2(void *unused) {//Responsavel por enviar um sinal a um processo
  int pid, sig;
  while(1) {
    scanf("%d %d", &pid, &sig);
    kill(pid, sig);
  }
}


int main() {
  srand(time(NULL));
  strcpy(pref, "/proc/");
  strcpy(suf, "/stat");
  pthread_t thread1;
  pthread_t thread2;
  if(pthread_create(&thread1, NULL, thread_func_1, NULL)) {//Uma thread fica responsavel por imprimir o top
    fprintf(stderr, "Erro ao criar thread\n");
    return 1;
  }
  if(pthread_create(&thread2, NULL, thread_func_2, NULL)) {//Uma responsavel por tratar o envio de sinais
    fprintf(stderr, "Erro ao criar thread\n");
    return 1;
  }
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  return 0;
}
