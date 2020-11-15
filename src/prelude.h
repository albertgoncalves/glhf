#ifndef __PRELUDE_H__
#define __PRELUDE_H__

typedef uint8_t  u8;
typedef uint32_t u32;

typedef int32_t i32;

typedef float  f32;
typedef double f64;

typedef FILE File;

typedef enum {
    FALSE = 0,
    TRUE = 1,
} Bool;

#define SIZE_BUFFER 512

typedef struct {
    char buffer[SIZE_BUFFER];
} Memory;

#define ERROR(x)                                                           \
    {                                                                      \
        fprintf(stderr, "%s:%s:%d %s\n", __FILE__, __func__, __LINE__, x); \
        exit(EXIT_FAILURE);                                                \
    }

static void set_file(Memory* memory, const char* filename) {
    File* file = fopen(filename, "r");
    if (!file) {
        ERROR("Unable to open file");
    }
    fseek(file, 0, SEEK_END);
    u32 file_size = (u32)ftell(file);
    if (sizeof(memory->buffer) <= file_size) {
        ERROR("sizeof(memory->buffer) <= file_size");
    }
    rewind(file);
    memory->buffer[file_size] = '\0';
    if (fread(&memory->buffer, sizeof(char), file_size, file) != file_size) {
        ERROR("`fread` failed");
    }
    fclose(file);
}

#endif
