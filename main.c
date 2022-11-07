// gcc -fopenmp ex5.c -o ex5
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <pthread.h>
#include <omp.h>

#define BYTES_PER_PAGE 1024
#define N_THREADS 12
#define FILENAME "/home/gilmar/CLionProjects/untitled/History64MB.txt"
#define LAST_PAGE 62544

struct Pair {
    char key[256];
    int value;
};

static struct Pair words_to_count[] = {
{
        .key =  "love",
        .value = 0,
},{
        .key =  "Shrek",
        .value = 0,
},{
        .key =  "SHREK", // Falas do SHREK
        .value = 0,
},{
        .key =  "onion",
        .value = 0,
},{
        .key =  "king",
        .value = 0,
}
};

static int n_words_to_count = 5;
pthread_mutex_t mutex;

struct timeval t1, t2;

int getWordIndex(void* word) {
//    printf("%s\n", (char *) word);

    for (int i = 0; i < n_words_to_count; i++) {
        if (!strcmp(words_to_count[i].key, word))
            return i;
    }

    return -1;
}

/*
void addWord(const char* word, int index) {
    struct Pair word_to_count = {
            .key =   word,
            .value = 0,
    };

    words_to_count[index] = word_to_count;
    n_words_to_count += 1;
}
*/

void sum_words(int id) {
    FILE* file = fopen(FILENAME, "r");

    int thread_last_page = (LAST_PAGE / N_THREADS) + 1;

    for (int current_page = 0; current_page < thread_last_page; ++current_page) {
        int offset = BYTES_PER_PAGE * (id + (current_page * N_THREADS));
        char page[BYTES_PER_PAGE];

        if (offset > 0) {
            fseek((FILE *) file, offset, SEEK_SET);
        }

        fgets(page, BYTES_PER_PAGE, (FILE *) file);

        char word[255] = "";
        for (int i = 0; i < BYTES_PER_PAGE; i++) {
            if (isalpha(page[i])) { // Adiciona caracteres a palavra
                char cToStr[2] = {page[i], '\000'};
                strcat(word, cToStr);
                continue;
            }

            int word_index = getWordIndex(&word);
            memset(word, '\0', sizeof word);

            if(word_index == -1) {
                if(page[i] == '\000') break;

                continue;
            }

            pthread_mutex_lock(&mutex);
//            printf("thread %i lock", id);
            words_to_count[word_index].value++;
//            printf("thread %i unlock\n", id);

            pthread_mutex_unlock(&mutex);

            if(page[i] == '\000') break;
        }
    }

    fclose((FILE *) file);
}

int main() {
    omp_set_num_threads(N_THREADS);
    gettimeofday(&t1, NULL);

    #pragma omp parallel for
    for (int i = 0; i < N_THREADS; i++)
        sum_words(i);

    gettimeofday(&t2, NULL);

    for (int i = 0; i < n_words_to_count; ++i) {
        struct Pair word = words_to_count[i];
        printf("Count %s: %i\n", word.key, word.value);
    }

    double t_total = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec) / 1000000.0);
    printf("\n=========================================\n");
    printf("Total Time = %f\n", t_total);
    printf("=========================================\n");
    return 0;
}
