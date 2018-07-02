#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>


#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2C_ACCELEROMETER_ADDRESS 0x1C

#define REG_IN 0x00
#define REG_ACTIVE_MODE 0x2A
#define REG_WRITE_ACTIVE 0X01

static pthread_t listeningThread;
static int i2cFileDesc;
static bool running;

// function declarations
static void* listenLoop (void *empty);
static int initI2cBus (char* bus, int address);
static int writeI2cReg (int i2cFileDesc, unsigned char regAddr, unsigned char value);
static unsigned char *readI2cReg (int i2CFileDesc, unsigned char regAddr, int *size);

// Start background thread for listening accelerometer
// return 0 for success
// return an error number for error
int Accelerometer_init(void)
{
    running = true;
    i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_ACCELEROMETER_ADDRESS);
    writeI2cReg(i2cFileDesc, REG_ACTIVE_MODE, REG_WRITE_ACTIVE);

    int threadCreateResult = pthread_create(&listeningThread, NULL, listenLoop, NULL);
    return threadCreateResult;
}

static void* listenLoop (void *empty)
{
    while (true){
        int size;
        unsigned char *list = readI2cReg(i2cFileDesc, REG_IN, &size);
        for (int i =0; i < 7; i++)
            printf("0x%02x ", list[i]);
        int x = ((int) list[1])*16 + ((int) list[2])/16;
        int y = list[3]*16 + list[4]/16;
        int z = list[5]*16 + list[6]/16;
        printf("x: %d, y: %d, z: %d\n", x, y, z);
        free(list);
        sleep(1);
    }

    return NULL;
}

// End the background thread
void Accelerometer_end(void)
{
    running = false;
    pthread_join(listeningThread, NULL);
    close(i2cFileDesc);
}

static int initI2cBus (char* bus, int address)
{
	int i2cFileDesc = open (bus, O_RDWR);
	if (i2cFileDesc < 0) {
		printf ("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
		perror ("Error is:");
		return -1;
	}

	int result = ioctl (i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) {
		perror ("Unable to set I2C device to slave address.");
		return -1;
	}
	return i2cFileDesc;
}

static int writeI2cReg (int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2) {
		perror ("Unable to write i2c register");
		return 1;
	}
    return 0;
}

// Need to free the returned array
static unsigned char *readI2cReg (int i2CFileDesc, unsigned char regAddr, int *size)
{
    *size = 7;

    // To read a register, must first write the address
    int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
    if (res != sizeof(regAddr)) {
    perror("I2C: Unable to write to i2c register.");
        exit(1);
    }

    // Now read the value and return it
    unsigned char *temp = malloc(sizeof(char)*7);
    res = read(i2cFileDesc, temp, sizeof(char)*7);

    if (res != sizeof(char)*7) {
        perror("I2C: Unable to read from i2c register");
        exit(1);
    }
    return temp;
}
