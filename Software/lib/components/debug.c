/* ----------------------------------------------------------------------------
 *          Mariokart project
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2011, University of Canterbury
 */

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
#include "debug.h"

//------------------------------------------------------------------------------
//         Global variable init
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Local Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Global Functions
//------------------------------------------------------------------------------
void debug_init(char *software_name){
    TRACE_CONFIGURE(DBGU_STANDARD, DEBUG_SPEED, BOARD_MCK);   
    //printf("-- Mariokart - %s Board %s --\n\r", SOFTWARE_NAME, SOFTPACK_VERSION);
    printf("-- Mariokart - %s Board %s --\n\r", software_name, SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
}
