#include "moving_average.h"

void moving_average_init(moving_average_t *average) {
  for (size_t i = 0; i < SMA_WINDOW_SIZE; ++i) {
    average->samples[i] = 0;
  }

  average->sum = 0;
  average->sample_count = 0;
  average->next_index = 0;
}

int moving_average_add(moving_average_t *average, int new_sample) {
  if (average->sample_count == SMA_WINDOW_SIZE) {
    // The buffer is full, so remove its oldest value from the sum.
    average->sum -= average->samples[average->next_index];
  } else {
    // During startup, average only the values that have actually been read.
    average->sample_count++;
  }

  // Store the new value in place of the oldest one and add it to the sum.
  average->samples[average->next_index] = new_sample;
  average->sum += new_sample;

  // Move to the next cell. After the last cell, return to the first one.
  average->next_index++;
  if (average->next_index == SMA_WINDOW_SIZE) {
    average->next_index = 0;
  }

  return average->sum / (int)average->sample_count;
}
