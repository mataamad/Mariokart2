#ifndef PID_H
#define PID_H

/**
PID gain calculator.
Note that this uses static variables for the I and D components
so pid_calculate_gain can only be used once on a board.

The proportional control calculation is stateless and so can be used as many times as desired
*/

/*Do a single step of a PID controller.
This function can only be used to calculate PID gains for one peripheral*/
int pid_calculate_gain(int desired, int current, double P, double I, double D);

/*do a single step of a proportional controller*/
int pid_calculate_P_gain(int desired, int current, double P);


#endif
