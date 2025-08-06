#ifndef FUNCTIONS_H
#define FUNCTIONS_H

int RI_request_objects(int RI_number_of_requested_objects, char **filenames); // Load object file(s)
int RI_init(int RI_window_width, int RI_window_height, char *RI_window_title); // Initialize RasterIver
int RI_stop(int result); // Stop RasterIver safely and free memory
int RI_tick(); // Tick RasterIver and render a frame

#endif