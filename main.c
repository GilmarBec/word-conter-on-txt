// gcc -o main main.c; ./main 8 0
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define true 1
#define false 0
#define BYTES_PER_PAGE 1024
//#define TEXT_SIZE 69763
#define IS_DEBUG_ON false
#define DEBUG if (IS_DEBUG_ON)
//#define N_PROCESS 8

struct Pair {
    char key[256];
    int value;
};
struct timeval t1, t2;
struct sockaddr_in address;
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
static int N_PROCESS = 8;
static char possible_files[2][256] = {"/home/gilmar/CLionProjects/untitled/History137KB.txt", "/home/gilmar/CLionProjects/untitled/History64MB.txt"};
static int TEXT_SIZE = 139526;
static char file_name[256];

void word_counter(int rank) {
    FILE* file = fopen(file_name, "r");

    for (int current_page = 0; true; ++current_page) {
        int offset = BYTES_PER_PAGE * (rank + (current_page * N_PROCESS));
        char page[BYTES_PER_PAGE];

        if (offset > 0) {
            if (offset > TEXT_SIZE) {
                break;
            }

            fseek((FILE *) file, offset, SEEK_SET);
        }

        fgets(page, BYTES_PER_PAGE, (FILE *) file);
        char text[BYTES_PER_PAGE] = "";
        for (int i = 0; i < BYTES_PER_PAGE; ++i) {
            char cToStr[2] = {page[i], '\000'};
            strcat(text, cToStr);
        }
        DEBUG printf("Rank[%i] Page[%i - %i]: %s\n", rank, offset, current_page, text);

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

    fclose((FILE *) file);
}

void sender(int pid) {
    int sockfd = (int) socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(8000);

    int result = connect(sockfd, (struct sockaddr *) &address, sizeof address);

    if(result == -1) {
        printf("Client failed: %i\n", pid);
        exit(1);
    }

    word_counter(pid);
    int msg[n_words_to_count];
    for (int i = 0; i < n_words_to_count; ++i) {
        msg[i] = words_to_count[i].value;
    }

    write(sockfd, &msg, sizeof msg);

    close(sockfd);
    exit(0);
}

void receiver() {
    struct sockaddr_in client_address;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(8000);

    bind(sockfd, (struct sockaddr *) &address, sizeof address);
    listen (sockfd, N_PROCESS);
    int msg[N_PROCESS][n_words_to_count];

    for (int i = 0; i < N_PROCESS; i++) {
        int client_len = sizeof client_address;
        int client_sockfd = accept(sockfd,(struct sockaddr *) &client_address, &client_len);
        memset(msg[i], 0, sizeof msg[i]);
        read(client_sockfd, &msg[i], sizeof msg[i]);
        close(client_sockfd);
    }

    for (int i = 0; i < N_PROCESS; ++i)
        for (int j = 0; j < n_words_to_count; ++j)
            words_to_count[j].value += msg[i][j];

    struct Pair word;
    for (int i = 0; i < n_words_to_count; ++i) {
        word = words_to_count[i];
        printf("%s: %i\n", word.key, word.value);
    }

    exit(0);
}

int main(int argc, char **argv) {
    sscanf(argv[1], "%d", &N_PROCESS);
    int j;
    sscanf(argv[2], "%d", &j);
    for (int i = 0; i < 256; ++i)
        file_name[i] = possible_files[j][i];
    if (j == 1) TEXT_SIZE = 64042434;

    gettimeofday(&t1, NULL);

    if (!(fork())) {
        receiver();
    }

    sleep(1);

    for (int i = 0; i < N_PROCESS; ++i) {
        if (!(fork())) {
            sender(i);
        }
    }

    int status;
    for (int i = 0; i < N_PROCESS + 1; ++i) {
        wait(&status);
    }

    gettimeofday(&t2, NULL);

    double t_total = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec) / 1000000.0);
    printf("\n=========================================\n");
    printf("Total Time = %f\n", t_total);
    printf("=========================================\n");
    return 0;
}
