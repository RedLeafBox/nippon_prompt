/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>

/* Standard includes. */
#include <stdio.h>

/*-----------------------------------------------------------*/
typedef enum  {
    rmv,
    renv4,
    rcmp1,
    rcmp2
} asicRegister;

static const int MIN_POSITION = -2147483648; 
static const int MAX_POSITION = 2147483648;
static const int REN4_VAL = 0x00003838;

static SemaphoreHandle_t asicSemaphore;
/*-----------------------------------------------------------*/

// task function definitions
static void monitorTask( void * parameters );
static void mainTask( void * parameters );

// fake functions for reading and writing registers
static void readRegister(int motorNumber, asicRegister reg, int val);
static void writeRegister(int motorNumber, asicRegister reg, int val);

/*-----------------------------------------------------------*/

static void readRegister(int motorNumber, asicRegister reg, int val) {
}

static void writeRegister(int motorNumber, asicRegister reg, int val) {
}

/*-----------------------------------------------------------*/

static void monitorTask( void * parameters )
{
    int positionMotor_1, positionMotor_2, positionMotor_3, positionMotor_4;
    int left, right;

    // obtain semaphore before communicating with the asic
    if(xSemaphoreTake(asicSemaphore, (TickType_t)1000 ) == pdTRUE ) {
        // initialize the env registers for all 4 motors according to documentation
        writeRegister(1, renv4, REN4_VAL);
        writeRegister(2, renv4, REN4_VAL);
        writeRegister(3, renv4, REN4_VAL);
        writeRegister(4, renv4, REN4_VAL);
           
        // return the semaphore
        xSemaphoreGive(asicSemaphore);
        } else {
            // could not obtain semaphore, error
    }

    for( ; ; )
    {
        // obtain semaphore before communicating with the asic
        if(xSemaphoreTake(asicSemaphore, (TickType_t)1000 ) == pdTRUE ) {

            // get the current positions of the motors
            readRegister(1, rmv, positionMotor_1);
            readRegister(2, rmv, positionMotor_2);
            readRegister(3, rmv, positionMotor_3);
            readRegister(4, rmv, positionMotor_4);

            // set the limits for all four motors, the limit being halfway to the next motor over
            left = MIN_POSITION;
            right = positionMotor_1 + (positionMotor_2 - positionMotor_1)/2;
            writeRegister(1, rcmp1, left);
            writeRegister(1, rcmp2, right);

            left = right;
            right = positionMotor_2 + (positionMotor_3 - positionMotor_2)/2;
            writeRegister(2, rcmp1, left);
            writeRegister(2, rcmp2, right);

            left = right;
            right = positionMotor_3 + (positionMotor_4 - positionMotor_3)/2;
            writeRegister(3, rcmp1, left);
            writeRegister(3, rcmp2, right);

            left = right;
            right = MAX_POSITION;
            writeRegister(4, rcmp1, left);
            writeRegister(4, rcmp2, right);

            // return the semaphore
            xSemaphoreGive(asicSemaphore);
        } else {
            // could not obtain semaphore, error
        }
        vTaskDelay(1000); /* delay 1000 ticks */
    }
}
/*-----------------------------------------------------------*/

// The main task receives user input, reads and writes to the asic registers,
// then sends data back to the user in a loop. 
static void mainTask( void * parameters )
{
    for( ; ; )
    {
        // Receive user input here

        // obtain semaphore before communicating with the asic
        if(xSemaphoreTake(asicSemaphore, (TickType_t)1000 ) == pdTRUE ) {

            // Read and write registers here

            // return the semaphore
            xSemaphoreGive(asicSemaphore);
        } else {
            // could not obtain semaphore, error
        }


        // Send data to user here

        // delay 100 ticks
        vTaskDelay( 100 );
    }
}
/*-----------------------------------------------------------*/


void main( void )
{
    static StaticTask_t monitorTaskTCB, mainTaskTCB;
    static StackType_t monitorTaskStack[ configMINIMAL_STACK_SIZE ], mainTaskStack[ configMINIMAL_STACK_SIZE ];

    // Create the main task 
    ( void ) xTaskCreateStatic( mainTask,
        "main",
        configMINIMAL_STACK_SIZE,
        NULL,
        configMAX_PRIORITIES - 1U,
        &( mainTaskStack[ 0 ] ),
        &( mainTaskTCB ) );

    // Create the task for monitoring the motor positions
    ( void ) xTaskCreateStatic( monitorTask,
        "monitor",
        configMINIMAL_STACK_SIZE,
        NULL,
        configMAX_PRIORITIES - 2U,
        &( monitorTaskStack[ 0 ] ),
        &( monitorTaskTCB ) );

    // Create a semaphore for the two tasks to share the asic
    asicSemaphore = xSemaphoreCreateMutex();
    if(NULL == asicSemaphore) {
    //ERROR
    }

    /* Start the scheduler. */
    vTaskStartScheduler();

    for( ; ; )
    {
        /* Should not reach here. */
    }
}
/*-----------------------------------------------------------*/

#if ( configCHECK_FOR_STACK_OVERFLOW > 0 )

    void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                        char * pcTaskName )
    {
        /* Check pcTaskName for the name of the offending task,
         * or pxCurrentTCB if pcTaskName has itself been corrupted. */
        ( void ) xTask;
        ( void ) pcTaskName;
    }

#endif /* #if ( configCHECK_FOR_STACK_OVERFLOW > 0 ) */
/*-----------------------------------------------------------*/

