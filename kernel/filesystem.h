// kernel/filesystem.h
// Simple RAM-based file system

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"

#define MAX_FILES 64
#define MAX_FILENAME_LEN 64
#define MAX_FILE_SIZE 8192

// File structure
typedef struct {
    char name[MAX_FILENAME_LEN];
    u8 data[MAX_FILE_SIZE];
    u32 size;
    bool exists;
} file_t;

// Filesystem structure
typedef struct {
    file_t files[MAX_FILES];
    u32 file_count;
} filesystem_t;

// Initialize filesystem
void fs_init(void);

// File operations
file_t* fs_create_file(const char* filename);
file_t* fs_open_file(const char* filename);
bool fs_write_file(file_t* file, const u8* data, u32 size);
bool fs_read_file(file_t* file, u8* buffer, u32 buffer_size, u32* bytes_read);
bool fs_delete_file(const char* filename);

// Directory operations
u32 fs_list_files(const char* path, char** filenames, u32 max_files);

// File info
bool fs_file_exists(const char* filename);
u32 fs_get_file_size(const char* filename);

#endif // FILESYSTEM_H
