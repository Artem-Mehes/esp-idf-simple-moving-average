#pragma once

#include <stddef.h>

#define SMA_WINDOW_SIZE 5

typedef struct {
  int samples[SMA_WINDOW_SIZE];
  int sum;
  size_t sample_count;
  size_t next_index;
} moving_average_t;

void moving_average_init(moving_average_t *average);
int moving_average_add(moving_average_t *average, int new_sample);
