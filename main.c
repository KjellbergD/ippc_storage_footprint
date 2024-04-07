#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <zconf.h>
#include <brotli/encode.h>
#include <brotli/decode.h>
#include "cJSON.h"
#include "pipeline_config.pb-c.h"  // Include the generated Protocol Buffers header file
#include "module_config.pb-c.h"

// Define a structure to represent module information
typedef struct {
    int order;
    char *name;
    int param_id;
} ModuleInfo;

// Define sample data
ModuleInfo modules[] = {
    {1, "preproc", 2},
    {2, "fextract", 4},
    {3, "classify", 1},
    {4, "postproc", 5},
    {5, "rgb64", 11},
    {6, "compress", 3},
    {7, "discr", 7},
    {8, "tiling", 8},
    {8, "superres", 8},
    {9, "resize", 9},
    {10, "clean", 10},
    {11, "rgb", 11},
    {12, "flip", 12},
    {13, "gray", 11},
    {14, "crc123", 15},
    {15, "crop", 2},
    {16, "testmod", 43},
    {17, "486comp", 3},
    {18, "recolor972", 14},
    {19, "filter", 1},
    {20, "format", 16},  
    {21, "normalize", 17},
    {22, "enhance", 18},
    {23, "denoise", 12},
    {24, "sharpen", 19},
    {25, "blur", 20},
    {26, "contrast", 13},
    {27, "saturation", 21},
    {28, "brightness", 22},
    {29, "cropresize", 23},
    {30, "rotate", 24},
    {31, "mirror", 25},
    {32, "watermark", 26},
    {33, "textoverlay", 27},
    {34, "colorbalance", 28},
    {35, "gamma", 29},
    {36, "threshold", 30},
    {37, "edgedetect", 31},
    {38, "emboss", 32},
    {39, "smoothing", 33},
    {40, "dilate", 34},  
};

typedef struct {
    char *key;
    int type;
    union {
        int bool_value;
        int int_value;
        float float_value;
        char *string_value;
    };
} ParamInfo;

ParamInfo params[] = {
    {"method", 5, string_value: "slow"},
    {"ratio", 4, float_value: 0.5},
    {"allow_loss", 2, bool_value: 1},
    {"retries", 3, int_value: 5},
    {"use_padding", 2, bool_value: 0},
    {"slices", 3, int_value: 25000},
    {"threshold", 4, float_value: 0.975},
    {"output_format", 5, string_value: "flattened"},
    {"size_limit", 4, float_value: 1024.5},
    {"enable_feature", 2, bool_value: 1},
    {"compression_level", 3, int_value: 8},
    {"encoding", 5, string_value: "utf-8"},
    {"timeout", 3, int_value: 300},
    {"quality", 4, float_value: 0.9},
    {"enable_logging", 2, bool_value: 1},
    {"max_connections", 3, int_value: 100},
    {"file_format", 5, string_value: "json"},
    {"precision", 4, float_value: 1e-6},
    {"enable_cache", 2, bool_value: 0},
    {"buffer_size", 3, int_value: 4096},
    {"pixel_depth", 3, int_value: 24},
    {"cache_timeout", 4, float_value: 15.2},
    {"debug_mode", 2, bool_value: 0},
    {"batch_size", 3, int_value: 64},
    {"learning_rate", 4, float_value: 0.001},
    {"use_dropout", 2, bool_value: 1},
    {"data_augmentation", 5, string_value: "enabled"},
    {"max_epochs", 3, int_value: 100},
    {"loss_function", 5, string_value: "crossentropy"},
    {"optimizer", 5, string_value: "adam"},
    {"shuffle_data", 2, bool_value: 1},
    {"validation_split", 4, float_value: 0.2},
    {"early_stopping", 2, bool_value: 1},
    {"weight_decay", 3, int_value: 0},
    {"activation_function", 5, string_value: "relu"},
    {"min_samples_split", 4, float_value: 2},
    {"use_bias", 2, bool_value: 1},
    {"kernel_size", 3, int_value: 3},
    {"stride_length", 4, float_value: 1},
    {"pooling_type", 5, string_value: "max"},
};

void pipeline_protobuf_size(int test_amount, size_t *out_raw, size_t *out_zlib, size_t *out_brotli) {
    // Protocol Buffers serialization
    PipelineDefinition p = PIPELINE_DEFINITION__INIT;
    p.n_modules = test_amount;
    p.modules = malloc(test_amount * sizeof(ModuleDefinition *));
    // Populate each ModuleDefinition
    for (size_t i = 0; i < test_amount; i++) {
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
    printf("[pipeline] Proto: %zu bytes\n", pb_size);
    *out_raw = pb_size;
    uint8_t packed[pb_size];
    pipeline_definition__pack(&p, packed);

    // Compress the protobuf
    uLongf compressed_size_proto = compressBound(pb_size);  // Calculate the size of compressed data
    Bytef *compressed_data_proto = (Bytef *)malloc(compressed_size_proto);
    if (compress(compressed_data_proto, &compressed_size_proto, (const Bytef *)packed, pb_size) != Z_OK) {
        printf("Compression failed.\n");
    }
    printf("[pipeline] Proto + zlib: %lu bytes\n", compressed_size_proto);
    *out_zlib = (size_t)compressed_size_proto;
    size_t encoded_buffer_size = 1000;
    uint8_t encoded_buffer[encoded_buffer_size];
    if (BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE, pb_size, (const uint8_t *)packed, &encoded_buffer_size, encoded_buffer) == BROTLI_FALSE)
    {
        fprintf(stderr, "Brotli compression failed\n");
    }
    printf("[pipeline] Proto + brotli: %lu bytes\n\n", encoded_buffer_size);
    *out_brotli = encoded_buffer_size;
}

void pipeline_json_size(int test_amount, size_t *out_raw, size_t *out_zlib, size_t *out_brotli) {
    // JSON serialization
    cJSON *root = cJSON_CreateArray();  // Create a JSON array
    for (size_t i = 0; i < test_amount; i++) {
        cJSON *module = cJSON_CreateObject();  // Create a JSON object for each module
        cJSON_AddNumberToObject(module, "order", modules[i].order);
        cJSON_AddStringToObject(module, "name", modules[i].name);
        cJSON_AddNumberToObject(module, "param_id", modules[i].param_id);
        cJSON_AddItemToArray(root, module);  // Add the module object to the array
    }
    char *json_string = cJSON_Print(root);  // Serialize JSON to string
    size_t json_size = strlen(json_string);  // Calculate the size of the JSON string
    cJSON_Delete(root);  // Free memory allocated by cJSON
    printf("[pipeline] JSON: %zu bytes\n", json_size);
    *out_raw = json_size;

    // Compress the JSON
    uLongf compressed_size_json = compressBound(json_size);  // Calculate the size of compressed data
    Bytef *compressed_data_json = (Bytef *)malloc(compressed_size_json);
    if (compress(compressed_data_json, &compressed_size_json, (const Bytef *)json_string, json_size) != Z_OK) {
        printf("Compression failed.\n");
    }
    printf("[pipeline] JSON + zlib: %lu bytes\n", compressed_size_json);
    *out_zlib = (size_t)compressed_size_json;
    size_t encoded_buffer_size = 1000;
    uint8_t encoded_buffer[encoded_buffer_size];
    if (BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE, json_size, (const uint8_t *)json_string, &encoded_buffer_size, encoded_buffer) == BROTLI_FALSE)
    {
        fprintf(stderr, "Brotli compression failed\n");
    }
    printf("[pipeline] JSON + brotli: %lu bytes\n\n", encoded_buffer_size);
    *out_brotli = encoded_buffer_size;
}

void pipeline_string_size(int test_amount, size_t *out_raw, size_t *out_zlib, size_t *out_brotli) {
    // String format serialization
    char string_buffer[1024];
    size_t string_size = 0;
    for (size_t i = 0; i < test_amount; i++) {
        string_size += snprintf(string_buffer + string_size, sizeof(string_buffer) - string_size,
                                "%d,%s,%d;", modules[i].order, modules[i].name, modules[i].param_id);
    }
    printf("[pipeline] String: %zu bytes\n", string_size);
    *out_raw = string_size;

    // Compress the string
    uLongf compressed_size_string = compressBound(string_size);
    Bytef *compressed_data_string = (Bytef *)malloc(compressed_size_string);
    if (compress(compressed_data_string, &compressed_size_string, (const Bytef *)string_buffer, string_size) != Z_OK) {
        printf("Compression failed.\n");
    }
    printf("[pipeline] String + zlib: %lu bytes\n", compressed_size_string);
    *out_zlib = (size_t)compressed_size_string;
    size_t encoded_buffer_size = 1000;
    uint8_t encoded_buffer[encoded_buffer_size];
    if (BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE, string_size, (const uint8_t *)string_buffer, &encoded_buffer_size, encoded_buffer) == BROTLI_FALSE)
    {
        fprintf(stderr, "Brotli compression failed\n");
    }
    printf("[pipeline] String + brotli: %lu bytes\n\n", encoded_buffer_size);
    *out_brotli = encoded_buffer_size;
}


void module_protobuf_size(int test_amount, size_t *out_raw, size_t *out_zlib, size_t *out_brotli) {
    // Protocol Buffers serialization
    ModuleConfig m = MODULE_CONFIG__INIT;
    m.n_parameters = test_amount;
    m.parameters = malloc(test_amount * sizeof(ConfigParameter *));
    // Populate each ModuleDefinition
    for (size_t i = 0; i < test_amount; i++) {
        // Allocate memory for the individual ModuleDefinition
        m.parameters[i] = malloc(sizeof(ConfigParameter));
        if (m.parameters[i] == NULL) {
            // Handle memory allocation failure
            exit(EXIT_FAILURE);
        }

        // Initialize the ModuleDefinition
        config_parameter__init(m.parameters[i]);
        m.parameters[i]->key = params[i].key;
        m.parameters[i]->value_case = params[i].type;
        switch (params[i].type)
        {
            case 2:
                m.parameters[i]->bool_value = params[i].bool_value;
                break;
            case 3:
                m.parameters[i]->int_value = params[i].int_value;
                break;
            case 4:
                m.parameters[i]->float_value = params[i].float_value;
                break;
            case 5:
                m.parameters[i]->string_value = strdup(params[i].string_value);
                break;
            default:
                break;
        }
    }
    size_t m_size = module_config__get_packed_size(&m);
    printf("[module] Proto: %zu bytes\n", m_size);
    *out_raw = m_size;
    uint8_t packed[m_size];
    module_config__pack(&m, packed);

    // Compress the protobuf
    uLongf compressed_size_proto = compressBound(m_size);  // Calculate the size of compressed data
    Bytef *compressed_data_proto = (Bytef *)malloc(compressed_size_proto);
    if (compress(compressed_data_proto, &compressed_size_proto, (const Bytef *)packed, m_size) != Z_OK) {
        printf("Compression failed.\n");
    }
    printf("[module] Proto + zlib: %lu bytes\n", compressed_size_proto);
    *out_zlib = (size_t)compressed_size_proto;
    size_t encoded_buffer_size = 1000;
    uint8_t encoded_buffer[encoded_buffer_size];
    if (BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE, m_size, (const uint8_t *)packed, &encoded_buffer_size, encoded_buffer) == BROTLI_FALSE)
    {
        fprintf(stderr, "Brotli compression failed\n");
    }
    printf("[module] Proto + brotli: %lu bytes\n\n", encoded_buffer_size);
    *out_brotli = encoded_buffer_size;
}

void module_json_size(int test_amount, size_t *out_raw, size_t *out_zlib, size_t *out_brotli) {
    // JSON serialization
    cJSON *root = cJSON_CreateArray();  // Create a JSON array
    for (size_t i = 0; i < test_amount; i++) {
        cJSON *config = cJSON_CreateObject();  // Create a JSON object for each module
        cJSON_AddStringToObject(config, "key", params[i].key);
        cJSON_AddNumberToObject(config, "type", params[i].type);
        switch (params[i].type)
        {
            case (2): 
                cJSON_AddBoolToObject(config, "bool_value", params[i].bool_value);
                break;
            case (3): 
                cJSON_AddNumberToObject(config, "int_value", params[i].int_value);
                break;
            case (4): 
                cJSON_AddNumberToObject(config, "float_value", params[i].float_value);
                break;
            case (5): 
                cJSON_AddStringToObject(config, "string_value", params[i].string_value);
                break;
        }
        cJSON_AddItemToArray(root, config);  // Add the module object to the array
    }
    char *json_string = cJSON_Print(root);  // Serialize JSON to string
    size_t json_size = strlen(json_string);  // Calculate the size of the JSON string
    cJSON_Delete(root);  // Free memory allocated by cJSON
    printf("[module] JSON: %zu bytes\n", json_size);
    *out_raw = json_size;

    // Compress the JSON
    uLongf compressed_size_json = compressBound(json_size);  // Calculate the size of compressed data
    Bytef *compressed_data_json = (Bytef *)malloc(compressed_size_json);
    if (compress(compressed_data_json, &compressed_size_json, (const Bytef *)json_string, json_size) != Z_OK) {
        printf("Compression failed.\n");
    }
    printf("[module] JSON + zlib: %lu bytes\n", compressed_size_json);
    *out_zlib = (size_t)compressed_size_json;
    size_t encoded_buffer_size = 1000;
    uint8_t encoded_buffer[encoded_buffer_size];
    if (BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE, json_size, (const uint8_t *)json_string, &encoded_buffer_size, encoded_buffer) == BROTLI_FALSE)
    {
        fprintf(stderr, "Brotli compression failed\n");
    }
    printf("[module] JSON + brotli: %lu bytes\n\n", encoded_buffer_size);
    *out_brotli = encoded_buffer_size;
}

void module_string_size(int test_amount, size_t *out_raw, size_t *out_zlib, size_t *out_brotli) {
    // String format serialization
    char string_buffer[1024];
    size_t string_size = 0;
    for (size_t i = 0; i < test_amount; i++) {
        switch (params[i].type)
        {
        case 2:
            string_size += snprintf(string_buffer + string_size, sizeof(string_buffer) - string_size,
                                "%s,%d,%d;", params[i].key, params[i].type, params[i].bool_value);
            break;
        case 3:
            string_size += snprintf(string_buffer + string_size, sizeof(string_buffer) - string_size,
                                "%s,%d,%d;", params[i].key, params[i].type, params[i].int_value);
            break;
        case 4:
            string_size += snprintf(string_buffer + string_size, sizeof(string_buffer) - string_size,
                                "%s,%d,%f;", params[i].key, params[i].type, params[i].float_value);
            break;
        case 5:
            string_size += snprintf(string_buffer + string_size, sizeof(string_buffer) - string_size,
                                "%s,%d,%s;", params[i].key, params[i].type, params[i].string_value);
            break;
        default:
            break;
        }
    }
    printf("[module] String: %zu bytes\n", string_size);
    *out_raw = string_size;

    // Compress the string
    uLongf compressed_size_string = compressBound(string_size);
    Bytef *compressed_data_string = (Bytef *)malloc(compressed_size_string);
    if (compress(compressed_data_string, &compressed_size_string, (const Bytef *)string_buffer, string_size) != Z_OK) {
        printf("Compression failed.\n");
    }
    printf("[module] String + zlib: %lu bytes\n", compressed_size_string);
    *out_zlib = (size_t)compressed_size_string;
    size_t encoded_buffer_size = 1000;
    uint8_t encoded_buffer[encoded_buffer_size];
    if (BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE, string_size, (const uint8_t *)string_buffer, &encoded_buffer_size, encoded_buffer) == BROTLI_FALSE)
    {
        fprintf(stderr, "Brotli compression failed\n");
    }
    printf("[module] String + brotli: %lu bytes\n\n", encoded_buffer_size);
    *out_brotli = encoded_buffer_size;
}


int main(int argc, char *argv[]) {

    int test_amount = 40;
    if (argc > 1)
    {
        test_amount = atoi(argv[1]);
    }

    FILE *fp;
    fp = fopen("sizes.txt", "w"); // Open file for writing
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    for (size_t i = 1; i <= test_amount; i++)
    {
        size_t s1, s2, s3, s4, s5, s6, s7, s8, s9;
        pipeline_protobuf_size(i, &s1, &s2, &s3);

        pipeline_json_size(i, &s4, &s5, &s6);

        pipeline_string_size(i, &s7, &s8, &s9);

        fprintf(fp, "%ld %zu %zu %zu %zu %zu %zu %zu %zu %zu\n", i, s1, s2, s3, s4, s5, s6, s7, s8, s9);

        module_protobuf_size(i, &s1, &s2, &s3);

        module_json_size(i, &s4, &s5, &s6);

        module_string_size(i, &s7, &s8, &s9);
        
        fprintf(fp, "%ld %zu %zu %zu %zu %zu %zu %zu %zu %zu\n", i, s1, s2, s3, s4, s5, s6, s7, s8, s9);
    }

    fclose(fp); // Close the file
    
    return 0;
}
