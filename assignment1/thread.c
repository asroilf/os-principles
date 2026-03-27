#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<time.h>

#define NUM_REPLICAS 3
#define MAX_READERS 15

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_read=PTHREAD_COND_INITIALIZER;
pthread_cond_t can_write=PTHREAD_COND_INITIALIZER;

int readers[NUM_REPLICAS] = {0,0,0};
int waiting_writers = 0;
int writer_active = 0;

char file_content[NUM_REPLICAS][256] = {
    "Initial content",
    "Initial content",
    "Initial content"
};

FILE *log_file;

/* for choosing replica with smallest number of readers */
int choose_replica() {
    int min = readers[0];
    int fileid = 0;

    for(int i=1;i<NUM_REPLICAS;i++){
        if(readers[i] < min){
            min = readers[i];
            fileid = i;
        }
    }

    return fileid;
}

/* logging function */
void logger(const char *agent, int file_name) {

    fprintf(log_file,"\n[%s]\n", agent);
    if(file_name!=-1)
        fprintf(log_file,"Accessed file: file%d.txt\n", file_name+1);

    fprintf(log_file, "Readers per file: [%d %d %d]\n",
        readers[0], readers[1], readers[2]);
    fprintf(log_file, "Writer active: %d\n", writer_active);
    fprintf(log_file, "File contents:\n");

    for(int i=0; i<NUM_REPLICAS; i++)
        fprintf(log_file,"file%d: %s\n", i+1, file_content[i]);

    fprintf(log_file,"---------------------------\n");
    fflush(log_file);
}

/* reader thread */
void* reader(void* arg){

    int rId = *((int*)arg);
    free(arg);

    pthread_mutex_lock(&mutex);

    // if writer is active OR writer is waiting,
    // reader must wait (writer priority)
    while(writer_active || waiting_writers > 0)
        pthread_cond_wait(&can_read,&mutex);

    int file_name = choose_replica();

    readers[file_name]++;

    pthread_mutex_unlock(&mutex);

    printf("Reader %d reading file%d\n", rId,file_name+1);
    sleep(rand()%3+1);
    pthread_mutex_lock(&mutex);

    --readers[file_name];
    logger("Reader",file_name);

    if(readers[file_name]==0)
        pthread_cond_signal(&can_write);

    pthread_mutex_unlock(&mutex);

    return NULL;
}

void* writer(void* arg){

    int version = 1;
    while(1){

        sleep(rand()%5+2);

        pthread_mutex_lock(&mutex);

        waiting_writers++;

        // writer must wait if another writer is active
        // or if any reader is currently reading any replica
        while(writer_active ||
              readers[0] + readers[1] + readers[2] > 0)
            pthread_cond_wait(&can_write,&mutex);

        waiting_writers--;
        writer_active = 1;

        pthread_mutex_unlock(&mutex);

        printf("Writer updating files\n");
        char new_content[256];
        sprintf(new_content,"Updated version %d",version++);
        for(int i=0;i<NUM_REPLICAS;i++)
            sprintf(file_content[i],"%s",new_content);
        sleep(2);

        pthread_mutex_lock(&mutex);

        logger("Writer",-1);

        pthread_cond_broadcast(&can_read);
        pthread_cond_signal(&can_write);
        pthread_mutex_unlock(&mutex);
        writer_active = 0;
    }

    return NULL;
}

int main(){

    srand(time(NULL));

    log_file = fopen("log.txt","w");

    pthread_t writer_thread;
    pthread_create(&writer_thread,NULL,writer,NULL);

    int reader_id = 1;

    while(reader_id <= MAX_READERS){

        sleep(rand()%3+1);

        pthread_t r;
        int *id = malloc(sizeof(int));
        *id = reader_id++;

        pthread_create(&r,NULL,reader,id);
        pthread_detach(r);
    }

    pthread_join(writer_thread,NULL);
    fclose(log_file);
}
