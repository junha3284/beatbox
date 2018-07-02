#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

// Start background thread for listening accelerometer
// return 0 for success
// return an error number for error
int Accelerometer_init(void);

// End the background thread
void Accelerometer_end(void);

#endif
