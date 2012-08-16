#include "pid.h"



/*Do a single step of a PID controller.
This function can only be used to calculate PID gains for one peripheral*/
int pid_calculate_gain(int desired, int current, double P, double I, double D) {
    static int err = 0;
    static int err_old = 0;
    static int I_err = 0;

    int P_err;
    int D_err;

    err_old = err;
    err = desired - current;

    P_err = err;
    I_err += err;

    D_err = err - err_old;

    int drive_percent = P * P_err + I * I_err + D*D_err;


    return drive_percent;
}

/*do a single step of a proportional controller*/
int pid_calculate_P_gain(int desired, int current, double P) {
    int error = desired - current;
    return P*error;
}
