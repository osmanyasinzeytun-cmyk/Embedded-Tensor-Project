#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* ---------------- TENSOR TYPE ---------------- */
typedef enum {
    TYPE_FLOAT32,
    TYPE_FLOAT16,
    TYPE_INT8
} TensorType;

/* ---------------- UNION DATA ---------------- */
typedef union {
    float*    f32_ptr;
    uint16_t* f16_ptr;
    int8_t*   i8_ptr;
} TensorData;

/* ---------------- TENSOR STRUCT ---------------- */
typedef struct {
    TensorType type;
    TensorData data;
    int size;
    float scale;
    int zero_point;
} DynamicTensor;

/* ---------------- CREATE ---------------- */
DynamicTensor create_tensor(TensorType type, int size) {
    DynamicTensor t;
    t.type = type;
    t.size = size;
    t.scale = 1.0f;
    t.zero_point = 0;

    if (type == TYPE_FLOAT32)
        t.data.f32_ptr = (float*)malloc((size_t)size * sizeof(float));

    else if (type == TYPE_FLOAT16)
        t.data.f16_ptr = (uint16_t*)malloc((size_t)size * sizeof(uint16_t));

    else
        t.data.i8_ptr = (int8_t*)malloc((size_t)size * sizeof(int8_t));

    return t;
}

/* ---------------- FREE ---------------- */
void free_tensor(DynamicTensor* t) {

    if (t == NULL) return;

    if (t->type == TYPE_FLOAT32 && t->data.f32_ptr != NULL)
        free(t->data.f32_ptr);

    else if (t->type == TYPE_FLOAT16 && t->data.f16_ptr != NULL)
        free(t->data.f16_ptr);

    else if (t->type == TYPE_INT8 && t->data.i8_ptr != NULL)
        free(t->data.i8_ptr);

    t->data.f32_ptr = NULL;
}

/* ---------------- FLOAT32 -> INT8 QUANT ---------------- */
void quantize_f32_to_int8(DynamicTensor* src, DynamicTensor* dst) {

    int i;
    float min, max;
    float qmin = -128.0f;
    float qmax = 127.0f;

    min = src->data.f32_ptr[0];
    max = src->data.f32_ptr[0];

    for(i = 1; i < src->size; i++) {
        if(src->data.f32_ptr[i] < min) min = src->data.f32_ptr[i];
        if(src->data.f32_ptr[i] > max) max = src->data.f32_ptr[i];
    }

    dst->scale = (max - min) / (qmax - qmin);
    if(dst->scale == 0.0f) dst->scale = 1.0f;

    dst->zero_point = (int)(qmin - min / dst->scale);

    for(i = 0; i < src->size; i++) {

        int q = (int)roundf(src->data.f32_ptr[i] / dst->scale) + dst->zero_point;

        if(q > 127) q = 127;
        if(q < -128) q = -128;

        dst->data.i8_ptr[i] = (int8_t)q;
    }
}

/* ---------------- DEQUANTIZE ---------------- */
float dequantize(int8_t q, float scale, int zero_point) {
    return ((float)q - (float)zero_point) * scale;
}

/* ---------------- FLOAT16 SIMULATION ---------------- */
uint16_t fake_float16(float value) {
    return (uint16_t)(value * 100.0f);
}

float fake_float16_to_f32(uint16_t v) {
    return (float)v / 100.0f;
}

/* ---------------- MAIN DEMO ---------------- */
int main(void) {

    int N = 5;
    int i;

    unsigned int mem_f32;
    unsigned int mem_i8;
    unsigned int mem_f16;

    float gain;

    DynamicTensor f32;
    DynamicTensor q8;
    DynamicTensor f16;

    printf("=== GOMULU SISTEM DINAMIK TENSOR DEMO ===\n\n");

    /* FLOAT32 DATA */
    f32 = create_tensor(TYPE_FLOAT32, N);

    printf("[Float32 Sensor Verisi]\n");
    for(i = 0; i < N; i++) {
        f32.data.f32_ptr[i] = 25.5f + (float)i;
        printf("%d -> %.3f\n", i, f32.data.f32_ptr[i]);
    }

    /* QUANTIZATION */
    q8 = create_tensor(TYPE_INT8, N);
    quantize_f32_to_int8(&f32, &q8);

    printf("\n[Quantized Int8 Veri]\n");
    for(i = 0; i < N; i++)
        printf("%d -> %d\n", i, q8.data.i8_ptr[i]);

    printf("\nScale: %.6f  ZeroPoint: %d\n", q8.scale, q8.zero_point);

    /* DEQUANT ERROR */
    printf("\n[Dequantization Error]\n");
    for(i = 0; i < N; i++) {

        float dq = dequantize(q8.data.i8_ptr[i], q8.scale, q8.zero_point);
        float err = fabsf(dq - f32.data.f32_ptr[i]);

        printf("%d -> original:%.3f restored:%.3f error:%.6f\n",
               i, f32.data.f32_ptr[i], dq, err);
    }

    /* FLOAT16 DEMO */
    f16 = create_tensor(TYPE_FLOAT16, N);

    printf("\n[Float16 Simulasyonu]\n");
    for(i = 0; i < N; i++) {

        float back;

        f16.data.f16_ptr[i] = fake_float16(f32.data.f32_ptr[i]);
        back = fake_float16_to_f32(f16.data.f16_ptr[i]);

        printf("%d -> stored:%u restored:%.3f\n", i, f16.data.f16_ptr[i], back);
    }

    /* MEMORY ANALYSIS */
    mem_f32 = (unsigned int)(N * sizeof(float));
    mem_i8  = (unsigned int)(N * sizeof(int8_t));
    mem_f16 = (unsigned int)(N * sizeof(uint16_t));

    printf("\n--- BELLEK ANALIZI ---\n");
    printf("Float32: %u byte\n", mem_f32);
    printf("Float16: %u byte\n", mem_f16);
    printf("Int8   : %u byte\n", mem_i8);

    gain = 100.0f * (1.0f - ((float)mem_i8 / (float)mem_f32));
    printf("Quantization Tasarruf: %.2f%%\n", gain);

    /* FREE */
    free_tensor(&f32);
    free_tensor(&q8);
    free_tensor(&f16);

    printf("\nProgram basariyla tamamlandi.\n");
    return 0;
}

