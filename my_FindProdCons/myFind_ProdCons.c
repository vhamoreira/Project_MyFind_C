/*
* Universidade Fernando Pessoa
* Sistemas Operativos
*
* myFind.c
*
* by:
* Eduardo Ribeiro (33812)
* Vitor Moreira (33953)
*/

#define _POSIX_SOURCE
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#undef _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <string.h>

#include <pthread.h>
#include <semaphore.h>

#define N 10
#define NProds 1
#define NCons 5
#define SIZE 100



typedef int (*PARAM)(struct dirent * entry, char * value);
int name (struct dirent * entry, char * value);
int iname (struct dirent * entry, char * value);
int type (struct dirent * entry, char * value);
int size(struct dirent * entry, char *value);
int mmin(struct dirent * entry, char *value);
int executable(struct dirent * entry, char *value);



const static struct
{
	const char *name;
	PARAM opt;
} function_map[] = {
	{"-name", name},
	{"-iname", iname},
	{"-type", type},
	{"-size", size},
	{"-mmin", mmin},
	{"-executable", executable},
};

typedef struct arg {
    PARAM opt;
    char * value;

}ARG;

typedef struct thread_data {
	pthread_t  thread_id;
    //path
	char *base_path;
    ARG args[50];
    int n_args;
    int n_thread;
}T_DATA;

void string_to_lower(char *data);
void erro();
char get_type( struct stat file);

char* buf[N];


int prodptr=0, consptr=0;

pthread_mutex_t trinco_p = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t trinco_c = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t trinco_write = PTHREAD_MUTEX_INITIALIZER;


static const char *semNameProd = "semPodeProd";
sem_t semPodeProd;

static const char *semNameCons = "semPodeCons";
sem_t semPodeCons;

int name (struct dirent * entry, char * value) {

       // printf("Find by name: %s\n", value);
       // printf("d_name: %s\n", entry->d_name);
       ///comparaçao com '*'
        int i,j;
        int count =0;
        int len = strlen(value);

        for(i = 0; i<len; i++){
            if(value[i] == '*'){
                for(j=0; j<i; j++){
                    if(entry->d_name[j] == value[j])
                        count++;
                }

                if (count == len-1)
                    return 1;
            }
        }

    ///comparação normal
    if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
        if(strcmp(entry->d_name, value) == 0){

        return 1; // return 1 if match found

        }

    }
    return 0;
}

int iname (struct dirent * entry, char * value) {

    char * word = malloc(sizeof(char) * 50);

    strcpy(word, entry->d_name);

    //printf("palavra: %s\n", entry->d_name);

    if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){

        string_to_lower(word);
        string_to_lower(value);

        if(strcmp(word, value) == 0){

        //printf("Find by name: %s\n", value);
        //printf("d_name: %s\n", entry->d_name);
        return 1; // return 1 if match found

        }

    }
    return 0;
}

int type (struct dirent * entry, char * value) {
    //printf("Find by type: %s\n", value);

    char type[5];
    strcpy(type, value);
    printf("char:%c\n", type[0]);

    struct stat file_stat;

    //printf("%s  tipo: %s",entry->d_name, type);

    if (stat(entry->d_name, &file_stat) == 0){

        if(type[0] == get_type(file_stat)){
            printf("tipo correspondodo: %c\n", get_type(file_stat));
            return 1;
        }
    }
    return 0;

}

int mmin (struct dirent * entry, char * value) {
    printf("Find by mmin: %s\n", value);

    struct stat file_stat;
    time_t aux;

    aux = time(NULL) + (60+atoi(value));


    if (stat(entry->d_name, &file_stat) == 0){
        if(file_stat.st_mode > aux){
            return 1;
        ///nao entra?
        printf("ola\n");

        }
    }

    return 0;

}


int executable(struct dirent * entry, char * value){
    printf("Find by executable:\n");

    struct stat file_stat;

    if(file_stat.st_mode == S_IXUSR || file_stat.st_mode == S_IXGRP || file_stat.st_mode == S_IXOTH);
        return 1;

    return 0;
}


int size(struct dirent * entry, char *value)
{
	printf("Find by size: %s\n", value);
	//todo
	return 1; // return 1 if match found
}


int find_func_id(char *arg)
{
    int i;

	for (i = 0; i < (sizeof(function_map) / sizeof(function_map[0])); i++)
	{
		if (!strcmp(function_map[i].name, arg) && function_map[i].opt)
		{
			return i;
		}
	}
	return -1;
}


void erro(){
	printf("Usage : find [path] [expression]\nexpression :\n-name <name>\n-type <type> (f,d,l,b,c,p,s)\n-executable\n-empty\n");
	exit(1);
}


void string_to_lower(char *data) {
	int i;
	for (i = 0; data[i] != '\0'; i++) {
		data[i] = tolower(data[i]);
	}
}


char get_type( struct stat file){

    ///file blocked
    if(S_ISBLK(file.st_mode)){
        return 'b';
    }
    ///character special file
    if(S_ISCHR(file.st_mode)){
        return 'c';
    }
    ///directory
    if(S_ISDIR(file.st_mode)){
        return 'd';
    }
    ///fifo
    if(S_ISFIFO(file.st_mode)){
        return 'p';
    }
    ///regular file
    if(S_ISREG(file.st_mode)){
        return 'f';
    }
    ///symbolic link
    if(S_ISLNK(file.st_mode)){
        return 'l';
    }
    /*
    if(S_ISSOCK(file.st_mode)){
        return 's';
    }
    */

    return'?';
}


void do_files(const char* base_path, struct dirent * entry, T_DATA t_data, struct stat *file_stat){

    int i;

    /*para 2 argumentos*/
    if(t_data.n_args>1){

        if(t_data.args[i].opt(entry, t_data.args[i].value) == 1 && t_data.args[i+1].opt(entry, t_data.args[i+1].value) == 1){
            printf("%s\n", base_path);
        }

    }

    /*para 1 argumento*/
    else{
        for (i = 0; i < t_data.n_args; i++){

            if(t_data.args[i].opt(entry, t_data.args[i].value) == 1){
                printf("%s\n", base_path);
            }
        }
    }
}



void * produtor(void * param)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) param;


    DIR *dir;
    struct dirent *entry;

    struct stat file_stat;
    //printf("original base_path: %s\n", base_path);

    char *path = malloc(sizeof(char) * 300);
    char *base_path = malloc(sizeof(char) * 300);

    strcpy(base_path, my_data->base_path);

        int i=0;
        int j;
        int k;
	//cast args

	T_DATA th_data;

    sem_wait(&semPodeProd);
    pthread_mutex_lock(&trinco_p);

    //printf("trinco produtor\n");
    buf[prodptr] = malloc(sizeof(char) * 300);
    strcpy(buf[prodptr], base_path);
    prodptr = (prodptr+1) % N;

    pthread_mutex_unlock(&trinco_p);
    sem_post(&semPodeCons);


    if ((dir = opendir(my_data->base_path)) == NULL)
    perror("opendir() error");
  else
  {
    //puts("contents of root:");

    while ((entry = readdir(dir)) != NULL)
    {

        if (my_data->base_path[strlen(my_data->base_path) - 1] == '/')
            sprintf(path, "%s%s", my_data->base_path, entry->d_name);
        else
            sprintf(path, "%s/%s", my_data->base_path, entry->d_name);

            //printf("paht: %s\n", path);
        //sprintf(path, "%s%s", base_path, entry->d_name);

        if ((strcmp(entry->d_name, ".")) == 0 || (strcmp(entry->d_name, "..")) == 0)
            continue;

      //printf("%s\n", path);
      if (stat(path, &file_stat) == 0)
      {

            //printf("n_args: %d", my_data->n_args);
            for(k=0; k<my_data->n_args; k++){
                th_data.args[k].opt = my_data->args[k].opt;
                th_data.args[k].value = my_data->args[k].value;

            }
                th_data.n_args = my_data->n_args;

            //printf("sou ficheiro\n");
            //do_files(path, entry, th_data ,&file_stat);

            if (S_ISDIR(file_stat.st_mode)) {
                //printf("diretorio:base_path: %s,  value:%s  n_thread: %d\n", my_data->base_path, th_data_array[k].args[0].value, th_data_array[k].n_thread);

                //do_files(path, entry, th_data_array[i] ,&file_stat);

                th_data.base_path = malloc(sizeof(char) * 300);

                strcat(path, "/");
                strcpy(th_data.base_path, path);
                //printf("diretorio:base_path: %s,  value:%s  n_thread: %d\n", th_data_array[i].base_path, th_data_array[i].args[0].value, th_data_array[i].n_thread);


                produtor(&th_data);
                //printf("enviei thread %lu|!!\n", (unsigned long)th_data_array[i].thread_id);
            }


      }

      strcpy(path, "");
    }
    closedir(dir);
  }

}

void * consumidor(void * param)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) param;

    T_DATA th_data;

    int k;

    DIR *dir;
    struct dirent *entry;

    struct stat file_stat;

    char *path = malloc(sizeof(char) * 300);
    char *base_path = malloc(sizeof(char) * 300);

    while(1) {
        int item;
        sem_wait(&semPodeCons);
        pthread_mutex_lock(&trinco_c);
        //printf("trinco consumidor\n");

            strcpy(path, buf[consptr]);
            buf[consptr] = NULL;
            consptr = (consptr+1) % N;

        pthread_mutex_unlock(&trinco_c);
        sem_post(&semPodeProd);

        ///consome();

    if ((dir = opendir(path)) == NULL)
        perror("opendir() error");
    else
    {
    //puts("contents of root:");

    while ((entry = readdir(dir)) != NULL)
    {

        if (path[strlen(path) - 1] == '/')
            sprintf(base_path, "%s%s", path, entry->d_name);
        else
            sprintf(base_path, "%s/%s", path, entry->d_name);

            //printf("paht: %s\n", path);
        //sprintf(path, "%s%s", base_path, entry->d_name);

        if ((strcmp(entry->d_name, ".")) == 0 || (strcmp(entry->d_name, "..")) == 0)
            continue;

      //printf("%s\n", path);
      if (stat(base_path, &file_stat) == 0)
      {

            //printf("n_args: %d", my_data->n_args);
            for(k=0; k<my_data->n_args; k++){
                th_data.args[k].opt = my_data->args[k].opt;
                th_data.args[k].value = my_data->args[k].value;

            }
                th_data.n_args = my_data->n_args;


            pthread_mutex_lock(&trinco_write);

            do_files(base_path, entry, th_data ,&file_stat);

            pthread_mutex_unlock(&trinco_write);



      }

      strcpy(path, "");
    }
    closedir(dir);
  }

    }

}




int main (int argc, char * argv[]) {

	int i=0, k=0, j;

    char *base_path = malloc(sizeof(char) * 300);


	T_DATA t_data = { .base_path="", .args={NULL, ""}, .n_args=0 };

    T_DATA th_data_array_prod[NProds];
    T_DATA th_data_array_cons[NCons];

    sem_init(&semPodeProd,0, N);
    sem_init(&semPodeCons,0, 0);


    for(i=0;i<NProds;i++){

    	th_data_array_prod[i].base_path= argv[1];

    	/*
        th_data_array_prod[i].n_args=0;

    	for(k=2; k<argc; k+=2){

        th_data_array_prod[i].args[th_data_array_prod[i].n_args].opt = function_map[find_func_id(argv[k])].opt;
        th_data_array_prod[i].args[th_data_array_prod[i].n_args].value = argv[k+1];
        th_data_array_prod[i].n_thread=0;
        th_data_array_prod[i].n_args++;
        }
        */

        //printf("value: %s, path: %s %d\n",th_data_array_prod[i].args[th_data_array_prod[i].n_args-1].value,th_data_array_prod[i].base_path, th_data_array_prod[i].n_args );
        pthread_create(&th_data_array_prod[i].thread_id, NULL, &produtor, &th_data_array_prod[i]);

    }

        for(i=0;i<NCons;i++){

    	th_data_array_cons[i].base_path= argv[1];
        th_data_array_cons[i].n_args=0;

    	for(k=2; k<argc; k+=2){

        th_data_array_cons[i].args[th_data_array_cons[i].n_args].opt = function_map[find_func_id(argv[k])].opt;
        th_data_array_cons[i].args[th_data_array_cons[i].n_args].value = argv[k+1];
        th_data_array_cons[i].n_thread=0;
        th_data_array_cons[i].n_args++;
        }

        //printf("value: %s, path: %s %d\n",th_data_array_cons[i].args[th_data_array_cons[i].n_args-1].value,th_data_array_cons[i].base_path, th_data_array_cons[i].n_args );
        pthread_create(&th_data_array_cons[i].thread_id, NULL, &consumidor, &th_data_array_cons[i]);
        //pthread_join (th_data_array_prod[i].thread_id, NULL);

    }

    for(j=0; j<NProds;j++){
        pthread_join (th_data_array_prod[j].thread_id, NULL);
        printf("\nthread_Prod:{%d}_id - %lu \n", j, (unsigned long)th_data_array_prod[j].thread_id);
    }

        for(j=0; j<NCons;j++){
        pthread_join (th_data_array_cons[j].thread_id, NULL);
        printf("\nthread_Cons:{%d}_id - %lu \n", j, (unsigned long)th_data_array_cons[j].thread_id);
    }

    /*
    for(i=0;i<NCons;i++)
        pthread_create(&th_data_array_cons[i].thread_id,NULL,&do_files,&th_data_array_cons[i]);
*/

   // pthread_create (&tid_state.thread_id, NULL, &do_directories, &tid_state);

    //pthread_join (tid_state.thread_id, NULL);


	//do_directories(t_data.base_path, t_data);
	return 0;
}
