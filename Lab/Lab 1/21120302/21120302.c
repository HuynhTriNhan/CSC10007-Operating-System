#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

// mutex để đảm bảo chỉ có 1 tiểu trình được thực thi tại 1 thời điểm
// sem để đảm bảo tiểu trình ghi trước tiểu trình đọc
sem_t sem, mutex;
char *file_name = "text.txt";
char *content = "21120302-HuynhTriNhan";

typedef struct
{
    char *filename;
    char *content;
} thread_args;

void *write_file(void *args)
{
    printf("Writing thread...\n");
    thread_args *a = (thread_args *)args;
    usleep(2000);
    sem_wait(&mutex);
    printf("\nThe writing thread is keeping mutex semaphore\n");
    // Test case: Xem xet neu tien trinh doc vao thi co chay duoc khong
    usleep(2000);
    FILE *f = fopen(a->filename, "w");
    fprintf(f, "%s", a->content);
    fclose(f);

    sem_post(&mutex);
    printf("\nThe writing thread gave back mutex semaphore\n");
    // Test case: Da tra mutex nhung chua tang sem len thi co chay duoc khong
    usleep(2000);
    sem_post(&sem);
    printf("\nWriting done! 'sem' variable semaphore increased\n");
    return NULL;
}

void *read_file(void *args)
{
    printf("Reading thread...\n");
    thread_args *a = (thread_args *)args;

    while (sem_trywait(&sem) != 0)
    {
        printf("Thread reading is waiting for thread writing ...\n");
        usleep(200);
    }
    printf("\nThe reading thread got a signal that The writing thread completed \n");

    sem_wait(&mutex);
    printf("\nThe reading thread is keeping mutex semaphore\n");
    FILE *f = fopen(a->filename, "r");
    char buffer[256];
    fscanf(f, "%s", buffer);
    printf("\nNoi dung doc duoc la: %s\n", buffer);
    fclose(f);

    sem_post(&mutex);
    printf("\nThe reading thread gave back mutex semaphore\n");
    sem_post(&sem);
    printf("\nReading done! 'sem' variable semaphore increased\n");
    return NULL;
}

int main()
{
    for (int i = 0; i < 3; i++)
    {
        printf("\n\nLan thu thu: %d\n", i + 1);
        sem_init(&mutex, 0, 1);
        sem_init(&sem, 0, 0);
        pthread_t pth_write, pth_read;

        thread_args args = {file_name, content};

        pthread_create(&pth_write, NULL, write_file, &args);
        pthread_create(&pth_read, NULL, read_file, &args);

        pthread_join(pth_write, NULL);
        pthread_join(pth_read, NULL);

        sem_destroy(&mutex);
        sem_destroy(&sem);
    }

    return 0;
}