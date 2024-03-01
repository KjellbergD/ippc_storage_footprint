#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "cJSON.h"
#include "pipeline_config.pb-c.h"  // Include the generated Protocol Buffers header file
#include <zconf.h>

// Define a structure to represent module information
typedef struct {
    int order;
    char name[50];
    int param_id;
} ModuleInfo;

// Define sample data
ModuleInfo modules[] = {
    {5, "m1", 2},
    {2, "m2", 6},
    {3, "m3", 1},
    {4, "m4", 4},
    {6, "m5", 5},
    {7, "m6", 7},
    {8, "m7", 8},
    {8, "m8", 8},
    {9, "m9", 9},
    {10, "m10", 10},
    {11, "m11", 11},
    {12, "m12", 12},
    // Add more sample data as needed
};

size_t num_modules = sizeof(modules) / sizeof(ModuleInfo);

void protobuf_size() {
    // Protocol Buffers serialization
    PipelineDefinition p = PIPELINE_DEFINITION__INIT;
    p.n_modules = num_modules;
    p.modules = malloc(num_modules * sizeof(ModuleDefinition *));
    // Populate each ModuleDefinition
    for (size_t i = 0; i < num_modules; i++) {
        // Allocate memory for the individual ModuleDefinition
        p.modules[i] = malloc(sizeof(ModuleDefinition));
        if (p.modules[i] == NULL) {
            // Handle memory allocation failure
            exit(EXIT_FAILURE);
        }

        // Initialize the ModuleDefinition
        module_definition__init(p.modules[i]);
        p.modules[i]->order = modules[i].order;
        p.modules[i]->name = strdup(modules[i].name);
        p.modules[i]->param_id = modules[i].param_id;
    }
    size_t pb_size = pipeline_definition__get_packed_size(&p);
    printf("Protocol Buffers data size: %zu bytes\n", pb_size);
    uint8_t packed[pb_size];
    pipeline_definition__pack(&p, packed);

    // Compress the protobuf
    uLongf compressed_size_proto = compressBound(pb_size);  // Calculate the size of compressed data
    Bytef *compressed_data_proto = (Bytef *)malloc(compressed_size_proto);
    if (compress(compressed_data_proto, &compressed_size_proto, (const Bytef *)packed, pb_size) != Z_OK) {
        printf("Compression failed.\n");
    }
    printf("Compressed Proto data size: %lu bytes\n\n", compressed_size_proto);
}

void json_size() {
    // JSON serialization
    cJSON *root = cJSON_CreateArray();  // Create a JSON array
    for (size_t i = 0; i < num_modules; i++) {
        cJSON *module = cJSON_CreateObject();  // Create a JSON object for each module
        cJSON_AddNumberToObject(module, "order", modules[i].order);
        cJSON_AddStringToObject(module, "name", modules[i].name);
        cJSON_AddNumberToObject(module, "param_id", modules[i].param_id);
        cJSON_AddItemToArray(root, module);  // Add the module object to the array
    }
    char *json_string = cJSON_Print(root);  // Serialize JSON to string
    size_t json_size = strlen(json_string);  // Calculate the size of the JSON string
    cJSON_Delete(root);  // Free memory allocated by cJSON
    printf("JSON data size: %zu bytes\n", json_size);

    // Compress the JSON
    uLongf compressed_size_json = compressBound(json_size);  // Calculate the size of compressed data
    Bytef *compressed_data_json = (Bytef *)malloc(compressed_size_json);
    if (compress(compressed_data_json, &compressed_size_json, (const Bytef *)json_string, json_size) != Z_OK) {
        printf("Compression failed.\n");
    }
    printf("Compressed JSON data size: %lu bytes\n\n", compressed_size_json);
}

void string_size() {
    // String format serialization
    char string_buffer[1024];
    size_t string_size = 0;
    for (size_t i = 0; i < num_modules; i++) {
        string_size += snprintf(string_buffer + string_size, sizeof(string_buffer) - string_size,
                                "%d,%s,%d;", modules[i].order, modules[i].name, modules[i].param_id);
    }
    printf("String data size: %zu bytes\n", string_size);

    // Compress the string
    uLongf compressed_size_string = compressBound(string_size);
    Bytef *compressed_data_string = (Bytef *)malloc(compressed_size_string);
    if (compress(compressed_data_string, &compressed_size_string, (const Bytef *)string_buffer, string_size) != Z_OK) {
        printf("Compression failed.\n");
    }
    printf("Compressed string data size: %lu bytes\n\n", compressed_size_string);
}

int main() {
    string_size();

    json_size();

    protobuf_size();

    return 0;
}
