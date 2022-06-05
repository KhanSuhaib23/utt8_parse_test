#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <windows.h>

#define UTF_SKEETO
#ifdef UTF_SKEETO
#include "utf8_skeeto.c"
#include "utf8_snk.c"
#include "utf8_hoehrmann.c"
#endif

uint8_t* file_open(const char* filename, FILE** file, uint64_t* sz) {
    *file = fopen(filename, "rb");


    fseek(*file, 0, SEEK_END);
    int32_t len = ftell(*file);
    fseek(*file, 0, SEEK_SET);

    uint8_t* buff = malloc(len + 16);
    fread(buff, len, 1, *file);
    for (size_t i = 0; i < 16; ++i) {
        buff[len + i] = 0;
    }

    *sz = len;

    return buff;
}

void file_close(uint8_t* buff, FILE* file) {
    fclose(file);
    free(buff);
}


void test_string(uint8_t* buff) {
    uint32_t code;
    uint32_t err;
    uint8_t* curr = buff;
    uint32_t code1;
    uint32_t err1;
    uint8_t* curr1 = buff;
    uint32_t errs = 0;
    volatile int32_t x;

    while (curr[0]) {
        curr = utf8_decode_dfa(curr, &code, &err);
        errs += err;

    }
    /* uint32_t v = utf8_validate_dfa(curr); */

    if (errs > 0) {
        x = errs;
    }

}

#define N 1

int main(int argc, char** argv) {
    LARGE_INTEGER li;
    int64_t fq;
    QueryPerformanceFrequency(&li);
    fq = li.QuadPart;


    {
        uint8_t* buff;
        FILE* file;
        uint64_t sz;
        double time = 0.0;
        int64_t t;

        buff = file_open("..\\data\\alice29.txt", &file, &sz);
        for (size_t i = 0; i < N; ++i) {
            QueryPerformanceCounter(&li);
            t = li.QuadPart;
            test_string(buff);

            QueryPerformanceCounter(&li);
            time += (li.QuadPart - t) * 1.0 / fq;
        }
        file_close(buff, file);

        printf("alice: %lfMB/s\n", (sz * 1.0 * N) / time / 1024.0 / 1024.0);
    }

    {
        uint8_t* buff;
        FILE* file;
        uint64_t sz;
        double time = 0.0;
        int64_t t;

        buff = file_open("..\\data\\sample_verify.txt", &file, &sz);
        for (size_t i = 0; i < N; ++i) {
            QueryPerformanceCounter(&li);
            t = li.QuadPart;
            test_string(buff);

            QueryPerformanceCounter(&li);
            time += (li.QuadPart - t) * 1.0 / fq;
        }
        file_close(buff, file);

        printf("sample: %lfMB/s\n", (sz * 1.0 * N) / time / 1024.0 / 1024.0);
    }
    
    return 0;
}
