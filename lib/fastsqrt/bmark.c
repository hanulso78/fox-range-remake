/* quick benchmarking util by Peter Wang 3 august 1998 */

#if 0
#include <stdio.h>
#include <math.h>
#include <allegro.h>  
#include "fastsqrt.h"

volatile int last_count = 0, counter = 0;

void persec()
{
    last_count = counter;
    counter = 0;
};
END_OF_FUNCTION(persec);

int main()
{
    int old = 0, anum = 0;

    allegro_init();
    LOCK_FUNCTION(persec);
    LOCK_VARIABLE(last_count);
    LOCK_VARIABLE(counter); 
    install_timer();
    install_int(persec, 1000);
    install_keyboard();

    build_sqrt_table();

    printf("\nStandard C sqrt():\n");

    while (!keypressed())
    {
	sqrt(anum++);
	counter++;

	if (old != last_count)
	{
	    printf("%d ", old = last_count);
	    fflush(stdout);
	}
    }
    clear_keybuf();

    printf("\n\nfast_sqrt():\n");

    while (!keypressed())
    {
	fast_fsqrt(anum++);
	counter++;

	if (old != last_count)
	{
	    printf("%d ", old = last_count);
	    fflush(stdout);
	}
    }
    clear_keybuf();

    printf("\n\nfast_sqrt_asm():\n");

    while (!keypressed())
    {
	fast_fsqrt_asm(anum++);
	counter++;

	if (old != last_count)
	{
	    printf("%d ", old = last_count);
	    fflush(stdout);
	}
    }
    clear_keybuf();
    
    printf("\n");

    return 0;
}

END_OF_MAIN();
#endif
