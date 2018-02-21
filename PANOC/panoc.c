/*
 * The panoc algorithm uses a combination of proximal gradient and lbfgs. The
 * linesearch is performed using an smoothed version of the problem called the
 * forward backward envelop, or short FBE.
 * 
 * Every iteration a linesearch(backtracking) on the combination is executed:
 * New direction = -(1-tau)*(direction gradient) + tau*(downward direction lbfgs)
 * 
 * tau=2^i with i as small as possible, we want to use as much lbfgs as possible so that
 * the linesearch condition FBE(new) = FBE(old) - constant*||direction||
 * 
 * Keep in mind that lbfgs has super linear convergence and proximal gradient has sub-linear 
 * convergence. In the beginning the proximal gradient is dominant, but after a while the lbfgs 
 * will be dominant.
 * 
 * More information on the panoc algorithm: ftp://ftp.esat.kuleuven.be/pub/stadius/athemeli/18-03.pdf 
 */
#include"panoc.h"
#include"math.h"
#include<stdlib.h>
#include"lbfgs.h"
#include"proximal_gradient_descent.h"
#include"matrix_operations.h"
#include"../globals/globals.h"
#include"casadi_interface.h"
#include"buffer.h"

#define INTERATION_INDEX_PURE_LBFGS_STEP 0

/* variables set once by init */
static size_t dimension;

/* variables reused by each get direction */
static real_t tau;
static real_t FBE_current_location;
static real_t direction_norm;

/* functions used internally */
static int panoc_check_linesearch_condition(int index_iteration,real_t* new_location, const real_t linesearch_gamma);
static int panoc_get_new_potential_location(const  real_t* forward_backward_step,
    const real_t* direction_residue,const real_t tau,real_t* potential_new_location);

/*
 * Initialize the panoc library
 * This function should allways be called before doing anything with the panoc lib
 */
int panoc_init(){
    dimension=casadi_interface_get_dimension();

    if(lbfgs_init(LBGFS_BUFFER_SIZE,dimension)==FAILURE) goto fail_1;
    if(proximal_gradient_descent_init()==FAILURE) goto fail_2;
    if(buffer_init()==FAILURE) goto fail_3;

    return SUCCESS;
    
    fail_3:
        proximal_gradient_descent_cleanup();
    fail_2:
        lbfgs_cleanup();
    fail_1:
        return FAILURE;
}

/*
 * cleanup the panoc library
 * This function cleans up the memory used by the panoc algorithm, 
 * use this when you don't need the lib anymore
 */
int panoc_cleanup(){
    buffer_cleanup();
    proximal_gradient_descent_cleanup();
    lbfgs_cleanup();
    return SUCCESS;
}

/*
 * Solve the actually MPC problem, return the optimal inputs
 */
real_t panoc_get_new_location(const real_t* current_location,real_t* new_location){  
    buffer_renew(current_location);
    const real_t* forward_backward_step = proximal_gradient_descent_get_direction(); /* in paper this is r*gamma */
    const real_t linesearch_gamma = proximal_gradient_descent_get_gamma();

    const real_t* direction_residue = lbfgs_get_direction();
    const real_t residual_inf_norm=proximal_gradient_descent_get_current_residual_inf_norm();

    /* precompute FBE used in linesearch check, static fields ! */
    if(residual_inf_norm>MACHINE_ACCURACY){
        FBE_current_location = proximal_gradient_descent_get_current_forward_backward_envelop();
        direction_norm=sq(vector_norm2(forward_backward_step,dimension));

        tau=1;
        panoc_get_new_potential_location(forward_backward_step,direction_residue,tau,new_location);
        int i=0;
        for(i=0;i<FBE_LINESEARCH_MAX_ITERATIONS && panoc_check_linesearch_condition(i,new_location,linesearch_gamma)==FAILURE;i++){
                tau=tau/2;
                panoc_get_new_potential_location(forward_backward_step,direction_residue,tau,new_location);
        }
        
        if(i==FBE_LINESEARCH_MAX_ITERATIONS){
            tau=0;
            panoc_get_new_potential_location(forward_backward_step,direction_residue,tau,new_location);
        }
    }

    return residual_inf_norm;
}

static int panoc_check_linesearch_condition(int index_iteration,real_t* new_location,const real_t linesearch_gamma){
    /*
     * If this is the first time you call the check_line_condition:
     *      use the precomputed FBE as your only using lbfgs
     */
    real_t FBE_potential_new_location;
    if(index_iteration==INTERATION_INDEX_PURE_LBFGS_STEP )
        FBE_potential_new_location = proximal_gradient_descent_get_lbfgs_forward_backward_envelop();
    else 
        FBE_potential_new_location = proximal_gradient_descent_forward_backward_envelop(new_location);

    const real_t factor = PROXIMAL_GRAD_DESC_SAFETY_VALUE/(4*linesearch_gamma);

    if(FBE_potential_new_location<=FBE_current_location-factor*direction_norm){
        /* 
         * SUCCESS means that the linesearch will stop, 
         * check if i should reuse something next iteration 
         */
        if(index_iteration==INTERATION_INDEX_PURE_LBFGS_STEP) /* linesearch succeeds with pure lbfgs step, reuse the function evaluation */
            buffer_set_lbfgs_as_precomputed();
        return SUCCESS; 
    }
    return FAILURE;
}

/* 
 * find potential new location x=x-(1-tau)*forward_backward_step+tau*direction_residue 
 */
static int panoc_get_new_potential_location(const  real_t* forward_backward_step,
    const real_t* direction_residue,const real_t tau,real_t* potential_new_location){

    const real_t* current_location = buffer_get_current_location();
    vector_add_2_vectors_a_times(current_location,forward_backward_step,direction_residue,dimension,\
        -(1-tau),tau,potential_new_location); 
    return SUCCESS;
}

real_t panoc_get_tau(void){return tau;}

/*
 * call this function from nmpc at the end to reset the buffers/counters
 */
int panoc_reset_cycli(void){
    proximal_gradient_descent_reset_iteration_counters();
    lbfgs_reset_iteration_counters();
    return SUCCESS;
}