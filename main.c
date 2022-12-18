// mpicc main.c -o main; mpiexec --hostfile hostfile -np 8 ./main
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <pthread.h>
#include <omp.h>
#include <mpi.h>
#include <stdlib.h>

#define true 1
#define false 0
#define BYTES_PER_PAGE 1024
//#define TEXT_SIZE 69763
#define IS_DEBUG_ON false
#define DEBUG if (IS_DEBUG_ON)

static char possible_files[2][256] = {"History137KB.txt", "History64MB.txt"};
static int TEXT_SIZE = 139526;
static char file_name[256];

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
},{
    .key = "locked",
    .value = 0,
},{
        .key = "dragon",
        .value = 0,
},{
        .key = "kiss",
        .value = 0,
},{
        .key = "ogre",
        .value = 0,
},{
        .key = "town",
        .value = 0,
},{
        .key = "eyes",
        .value = 0,
},{
        .key = "men",
        .value = 0,
},{
        .key = "torches",
        .value = 0,
},{
        .key = "down",
        .value = 0,
},{
        .key = "Fairy",
        .value = 0,
},{
        .key = "tale",
        .value = 0,
},{
        .key = "creatures",
        .value = 0,
},{
        .key = "guard",
        .value = 0,
},{
        .key = "cages",
        .value = 0,
},{
        .key = "farmer",
        .value = 0,
},{
        .key = "people",
        .value = 0,
},{
        .key = "GUARD",
        .value = 0,
},{
        .key = "rope",
        .value = 0,
},{
        .key = "have",
        .value = 0,
},{
        .key = "little",
        .value = 0,
},{
        .key = "chance",
        .value = 0,
},{
        .key = "change",
        .value = 0,
},{
        .key = "Please",
        .value = 0,
},{
        .key = "Donkey",
        .value = 0,
},{
        .key = "Guards",
        .value = 0,
},{
        .key = "shut",
        .value = 0,
},{
        .key = "want",
        .value = 0,
},{
        .key = "really",
        .value = 0,
},{
        .key = "freak",
        .value = 0,
},{
        .key = "Farquaad",
        .value = 0,
},{
        .key = "lord",
        .value = 0,
},{
        .key = "Lord",
        .value = 0,
},{
        .key = "GINGERBREAD",
        .value = 0,
},{
        .key = "Gingerbread",
        .value = 0,
},{
        .key = "field",
        .value = 0,
},{
        .key = "line",
        .value = 0,
},{
        .key = "keep",
        .value = 0,
},{
        .key = "champion",
        .value = 0,
},{
        .key = "cheers",
        .value = 0,
},{
        .key = "larger",
        .value = 0,
},{
        .key = "roll",
        .value = 0,
},{
        .key = "beer",
        .value = 0,
},{
        .key = "rude",
        .value = 0,
},{
        .key = "ROBIN",
        .value = 0,
},{
        .key = "boys",
        .value = 0,
}
};

static int n_words_to_count = 10;

struct timeval t1, t2;

#define SEND {\
    int receiver = 0; \
    MPI_Bcast(&word_counters, n_words_to_count, MPI_INT, rank, MPI_COMM_WORLD);\
}

#define RECEIVE {\
    for(int sender_i = 0; sender_i < world_size; sender_i++){ \
        if(sender_i == rank) {\
            continue;\
        }\
        MPI_Bcast(&word_counters, n_words_to_count, MPI_INT, sender_i, MPI_COMM_WORLD);\
        for (i = 0; i < n_words_to_count; i++)\
            words_to_count[i].value += word_counters[i];\
    }\
}
//MPI_Barrier(MPI_COMM_WORLD);

int main(int argc, char **argv) {
    int i, j;
    sscanf(argv[1], "%d", &j);
    for (i = 0; i < 256; ++i)
        file_name[i] = possible_files[j][i];
    if (j == 1) TEXT_SIZE = 64042434;

    MPI_Init(NULL, NULL);                       // INIT

    gettimeofday(&t1, NULL);
    int world_size, rank;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // N of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // MPI id
    MPI_File	file;
    MPI_Status	status;
    printf("text\n");

    //int bufsize = TEXT_SIZE/world_size;
    //int nints = bufsize;
    //char buf[BYTES_PER_PAGE] = "";
    //memset(buf, '\0', sizeof buf);
//
    //MPI_File_open(MPI_COMM_WORLD, file_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    //MPI_File_seek(file, rank * bufsize, MPI_SEEK_SET);
    //MPI_File_read(file, buf, 1, MPI_BYTE, &status);
    //MPI_File_read(file, buf, 1, MPI_BYTE, &status);
    //MPI_File_close(&file);
    //printf("text[rank-%i]:%s\n", rank, buf);
    //MPI_Finalize();                             // End MPI process
    //exit(0);

    MPI_File_open(MPI_COMM_WORLD, file_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    for (int current_page = 0; true; ++current_page) {
        int offset = BYTES_PER_PAGE * (rank + (current_page * world_size));
        char page[BYTES_PER_PAGE];

        if (offset > 0) {
            if (offset >= TEXT_SIZE)
                break;
            MPI_File_seek(file, offset, MPI_SEEK_SET);
        }

        MPI_File_read(file, page, BYTES_PER_PAGE, MPI_BYTE, &status);
        char text[BYTES_PER_PAGE] = "";
        for (i = 0; i < BYTES_PER_PAGE; ++i) {
            char cToStr[2] = {page[i], '\000'};
            strcat(text, cToStr);
        }
        DEBUG printf("Rank[%i] Page[%i - %i]: %s\n", rank, offset, current_page, text);

        char word[255] = "";
        for (i = 0; i < BYTES_PER_PAGE; i++) {
            if (isalpha(page[i])) { // Adiciona caracteres a palavra
                char cToStr[2] = {page[i], '\000'};
                strcat(word, cToStr);
                continue;
            }

            // Word index
            int word_index = -1;
            for (j = 0; j < n_words_to_count; j++) {
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

    for (int word_counter_index = 0; word_counter_index < n_words_to_count; word_counter_index++)
        word_counters[word_counter_index] = words_to_count[word_counter_index].value;

    SEND
    RECEIVE

    MPI_File_close(&file);
    MPI_Finalize();                             // End MPI process

    gettimeofday(&t2, NULL);

    double t_total = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec) / 1000000.0);
    printf("\n=========================================\n");
    printf("Total Time = %f\n", t_total);
    printf("=========================================\n");

    char msg[5000] = "";
    sprintf(msg, "RANK[%i] \n\n", rank);

    for (i = 0; i < n_words_to_count; ++i) {
        struct Pair word = words_to_count[i];
        char tmp[300] = "";
        sprintf(tmp, "Count %s: %i\n", word.key, word.value);
        strcat(msg, tmp);
    }

    printf("%s", msg);
    return 0;
}
