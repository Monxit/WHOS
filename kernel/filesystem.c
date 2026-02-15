// kernel/filesystem.c
// Simple RAM-based file system implementation

#include "filesystem.h"
#include "string.h"

static filesystem_t fs;

void fs_init(void) {
    fs.file_count = 0;
    for (u32 i = 0; i < MAX_FILES; i++) {
        fs.files[i].exists = false;
        fs.files[i].size = 0;
    }
}

file_t* fs_create_file(const char* filename) {
    if (fs.file_count >= MAX_FILES) return NULL;
    
    // Find empty slot
    for (u32 i = 0; i < MAX_FILES; i++) {
        if (!fs.files[i].exists) {
            file_t* file = &fs.files[i];
            
            // Copy filename
            u32 len = 0;
            while (filename[len] && len < MAX_FILENAME_LEN - 1) {
                file->name[len] = filename[len];
                len++;
            }
            file->name[len] = '\0';
            
            // Initialize file
            file->size = 0;
            file->exists = true;
            fs.file_count++;
            
            return file;
        }
    }
    return NULL;
}

file_t* fs_open_file(const char* filename) {
    for (u32 i = 0; i < MAX_FILES; i++) {
        if (fs.files[i].exists) {
            bool match = true;
            for (u32 j = 0; j < MAX_FILENAME_LEN; j++) {
                if (fs.files[i].name[j] != filename[j]) {
                    match = false;
                    break;
                }
                if (filename[j] == '\0') break;
            }
            if (match) {
                return &fs.files[i];
            }
        }
    }
    return NULL;
}

bool fs_write_file(file_t* file, const u8* data, u32 size) {
    if (!file || !file->exists || size > MAX_FILE_SIZE) return false;
    
    // Copy data
    for (u32 i = 0; i < size; i++) {
        file->data[i] = data[i];
    }
    file->size = size;
    return true;
}

bool fs_read_file(file_t* file, u8* buffer, u32 buffer_size, u32* bytes_read) {
    if (!file || !file->exists || !buffer) return false;
    
    u32 to_read = file->size;
    if (to_read > buffer_size) to_read = buffer_size;
    
    for (u32 i = 0; i < to_read; i++) {
        buffer[i] = file->data[i];
    }
    
    if (bytes_read) *bytes_read = to_read;
    return true;
}

bool fs_delete_file(const char* filename) {
    file_t* file = fs_open_file(filename);
    if (!file) return false;
    
    file->exists = false;
    file->size = 0;
    fs.file_count--;
    return true;
}

u32 fs_list_files(const char* path, char** filenames, u32 max_files) {
    (void)path; // Ignore path for now, just list all files
    
    u32 count = 0;
    for (u32 i = 0; i < MAX_FILES && count < max_files; i++) {
        if (fs.files[i].exists) {
            filenames[count] = fs.files[i].name;
            count++;
        }
    }
    return count;
}

bool fs_file_exists(const char* filename) {
    return fs_open_file(filename) != NULL;
}

u32 fs_get_file_size(const char* filename) {
    file_t* file = fs_open_file(filename);
    return file ? file->size : 0;
}
