#include "stdlib.h"
#include "../include/nmpc.h"
#include "../globals/globals.h"
#include "timer.h"

void simulation_init();
void simulation_cleanup();
const struct Panoc_time simulate_nmpc_panoc(real_t* current_state,real_t* optimal_inputs);
/*
 * Simulates the controller and fill optimal_input with the optimal input.
 * -> returns the time till convergence
 */
void simulation_init(){
    nmpc_init();
}
const struct Panoc_time simulate_nmpc_panoc(real_t* current_state,real_t* optimal_inputs){
    panoc_timer_start();

    npmc_solve(current_state,optimal_inputs);

    const struct Panoc_time time_difference = panoc_timer_stop();

    return time_difference;
}
void simulation_cleanup(){
    nmpc_cleanup();
}