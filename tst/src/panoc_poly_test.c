#include<stdio.h>
#include<math.h>
#include"../../PANOC/panoc.h"
#include"../../PANOC/matrix_operations.h"
#include"../../globals/globals.h"
#include<stdlib.h>

#define DIMENSION 2
static const real_t theoretical_solution[]={0,0};
static int degree=5;
static int w=0;
int checkIfSolutionIsReached(void);

void print_location(const real_t* location);

/*
 * Function f with its gradient df  
 * Function g with the resulting function after proximal operator
 */
real_t f_panoc_poly_test(const real_t* x);
real_t g_panoc_poly_test(const real_t* x);
void df_panoc_poly_test(const real_t* x ,real_t* df_x);
void proxg_panoc_poly_test(const real_t* x ,real_t* proxg_x);

/*
 * TEST proximal gradient descent
 * polynomial f(x1,x2,x3) = x1^4+x2^4+ x3^4+x1^3+x2^3+ x3^3
 * f(x) =0 if x=[0 0 0 0]
 */
int main(){
    return checkIfSolutionIsReached();
}

int checkIfSolutionIsReached(void){
    printf("test1 --- \n");
    degree=5;
    size_t numer_of_iterations=10;
    
    real_t* current_location=malloc(DIMENSION*sizeof(real_t));
    current_location[0]=0.5;current_location[1]=0.5;
    real_t* next_location=malloc(DIMENSION*sizeof(real_t));

    printf("starting in location x1=0.5 x2=0.5 \n");
    panoc_init(DIMENSION,\
        g_panoc_poly_test,\
        proxg_panoc_poly_test,\
        f_panoc_poly_test,\
        df_panoc_poly_test);

    size_t i;
    for ( i = 0; i < numer_of_iterations; i++)
    {
            panoc_get_new_location(current_location,next_location);
            /* move the next location to the current location */
            real_t* buffer=current_location;
            current_location=next_location;
            next_location=buffer;
            /* print out the location */
            print_location(current_location);
    }
    free(current_location);
    free(current_location);
    panoc_cleanup();

    if(current_location[0]<0.14){ /* theoretical value is about 0.133333 */
        printf("end of test1:SUCCESS --- \n");
        return SUCCESS;
    }else{
        printf("end of test1:FAILURE --- \n");
        return FAILURE;
    }  
}

/*
 * simple problem to test the proximal gradient descent
 */
real_t f_panoc_poly_test(const real_t* x){
    real_t f_x=0;
    size_t i;
    for (i = 0; i < DIMENSION; i++){
        f_x+=pow(x[i],degree);
    }
    return f_x;
}
real_t g_panoc_poly_test(const real_t* x){
    real_t g_x=0;
    size_t i;
    /* g is unused in this test so we leave this blank */
    return g_x;
}
void df_panoc_poly_test(const real_t* x ,real_t* df_x){
    size_t i;
    for (i = 0; i < DIMENSION; i++){
        df_x[i] = degree*pow(x[i],degree-1) ;
    }
}
real_t sign(real_t x){
    if(x>=0)return 1;
    else return -1;
}
void proxg_panoc_poly_test(const real_t* x ,real_t* proxg_x){
    size_t i;
    for (i = 0; i < DIMENSION; i++){
        if(fabs(x[i])<=w){
            proxg_x[i]=x[i];
        }else if(fabs(x[i])>2*w){
            proxg_x[i]=sign(x[i])*(fabs(x[i])-w);
        }else{
            proxg_x[i]=sign(x[i])*(fabs(x[i]-w));
        }
    }
}
// function [ prox_g_x ] = prox_g( x,w,dimension )
// prox_g_x=zeros(dimension,1);
// for i=1:dimension
//    if(abs(x(i))<w)
//        prox_g_x(i) = x(i);
//    elseif(abs(x)>2*w)
//        prox_g_x(i) = sign(x(i))*(abs(x(i))-w);
//    else
//        prox_g_x(i) = sign(x(i))*(abs(x(i)-w));
//    end
// end
// end
void print_location(const real_t* location){
    printf("x1=%f x2=%f \n",location[0],location[1]);
}