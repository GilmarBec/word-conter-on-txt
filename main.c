// mpicc main.c -o main; mpiexec --hostfile hostfile -np 8 ./main
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <pthread.h>
#include <omp.h>
#include <mpi.h>

#define true 1
#define false 0
#define BYTES_PER_PAGE 1024
#define FILENAME "/home/gilmar/CLionProjects/untitled/History69KB.txt"
#define TEXT_SIZE 69763
#define DEBUG false

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

struct timeval t1, t2;

#define SEND {\
    int receiver = 0; \
    MPI_Send(&word_counters, n_words_to_count, MPI_INT, receiver, 0, MPI_COMM_WORLD);\
}

#define RECEIVE {\
    for(int sender_i = world_size - 1; sender_i > 0; sender_i--){ \
        MPI_Recv(&word_counters, n_words_to_count, MPI_INT, sender_i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); \
        for (int i = 0; i < n_words_to_count; i++)\
            words_to_count[i].value += word_counters[i];\
    }\
}

int main() {
    gettimeofday(&t1, NULL);
    MPI_Init(NULL, NULL);                       // INIT
    int world_size, rank;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // N of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // MPI id
    FILE* file = fopen(FILENAME, "r");

    for (int current_page = 0; true; ++current_page) {
        int offset = BYTES_PER_PAGE * (rank + (current_page * world_size));
        char page[BYTES_PER_PAGE];

        if (offset > 0) {
            if (offset >= TEXT_SIZE)
                break;
            fseek((FILE *) file, offset, SEEK_SET);
        }

        fgets(page, BYTES_PER_PAGE, (FILE *) file);
        char text[BYTES_PER_PAGE] = "";
        for (int i = 0; i < BYTES_PER_PAGE; ++i) {
            char cToStr[2] = {page[i], '\000'};
            strcat(text, cToStr);
        }
        if (DEBUG) printf("Rank[%i] Page[%i - %i]: %s\n", rank, offset, current_page, text);

        char word[255] = "";
        for (int i = 0; i < BYTES_PER_PAGE; i++) {
            if (isalpha(page[i])) { // Adiciona caracteres a palavra
                char cToStr[2] = {page[i], '\000'};
                strcat(word, cToStr);
                continue;
            }

            // Word index
            int word_index = -1;
            for (int j = 0; j < n_words_to_count; j++) {
                if (!strcmp(words_to_count[j].key, word))
                    word_index = j;
            }
            //Fim Word index

            memset(word, '\0', sizeof word);

            if(word_index == -1) {
                if(page[i] == '\000') break;

                continue;
            }

            words_to_count[word_index].value++;

            if(page[i] == '\000') break;
        }
    }

    int word_counters[n_words_to_count];
    if (rank != 0) {
        for (int word_counter_index = 0; word_counter_index < n_words_to_count; word_counter_index++)
            word_counters[word_counter_index] = words_to_count[word_counter_index].value;

        SEND
    } else {
        RECEIVE
    }

    fclose((FILE *) file);
    MPI_Finalize();                             // End MPI process

    gettimeofday(&t2, NULL);

    if (rank != 0)
        return 0;

    double t_total = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec) / 1000000.0);
    printf("\n=========================================\n");
    printf("Total Time = %f\n", t_total);
    printf("=========================================\n");

    for (int i = 0; i < n_words_to_count; ++i) {
        struct Pair word = words_to_count[i];
        printf("Count %s: %i\n", word.key, word.value);
    }
    return 0;
}
