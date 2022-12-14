#include "vas_airAbsorption.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
	printf("%f", T0);
	vas_airAbsorptionFilter *x;
	x = vas_airAbsorbtionFilter_new(44100);
	/*x->t = 20 + T0;
	x->p = P0;
	x->h = 2*H0;
	//vas_airAbsorptionFilter_update_f_N(x);
	//vas_airAbsorptionFilter_update_f_O(x);
	vas_airAbsorptionFilter_udpate_f_O_N(x);
	//filterCutoffSolve(x);
	printf("%f\n", x->f_N);
	printf("%f\n", x->f_O);
	printf("%f", vas_airAbsorptionFilter_get_abs_coeff(x, 10000));*/

	return 0;
}