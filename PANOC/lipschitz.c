#include"lipschitz.h"
#include"matrix_operations.h"
#include"math.h"
#include"casadi_interface.h"
#include <stddef.h>
#include <stdlib.h>
#include "buffer.h"
/*
 * Estimate the lipschitz constant by using the numerical hessian as an estimation
 * Theorem:
 *     ||gradient(x)|| < B
 *     f is B-lipschitz
 */
real_t get_lipschitz(const real_t* current_position){
    const size_t dimension=casadi_interface_get_dimension();

    real_t current_position_delta[dimension];
    real_t df_current_position_delta[dimension];

    /* copy over the current position and add delta*/
    vector_real_add(current_position,dimension,DELTA_LIPSCHITZ,current_position_delta);

    /* calculate the two gradients and save them in the same variable */
    const real_t* df_current_position=buffer_get_current_df();
    casadi_interface_df(current_position_delta,df_current_position_delta);

    /* 
     * L = norm((df(x+delta)-df(x))/delta) 
     * reuse the current_position_delta values as buffer
     */
    vector_sub(df_current_position,df_current_position_delta,dimension,current_position_delta); /* step1: df(x+delta)-df(x) */
    vector_real_mul(current_position_delta,dimension,1/DELTA_LIPSCHITZ,current_position_delta); /* step2: df(x+delta)-df(x))/delta */
    return vector_norm2(current_position_delta,dimension); /* step3: norm((df(x+delta)-df(x))/delta) */
}