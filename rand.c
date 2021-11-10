#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

double rand_gen();
double normalRandom();

double rand_gen() {
   // return a uniformly distributed random value
   return ( (double)(rand()) + 1. )/( (double)(RAND_MAX) + 1. );
}

double normalRandom() {
   // return a normally distributed random value
   double v1=rand_gen();
   double v2=rand_gen();
   return cos(2*3.14*v2)*sqrt(-2.*log(v1));
}

 int main() {
   double sigma = 1;
   double Mi = 0;
   for(int i=0;i<20;i++) {
      double x = normalRandom()*sigma+Mi;
      printf("error: %f", x);

   }
   return 0;
}