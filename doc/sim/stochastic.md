# Stochastic Events in Cytosim
 
Most stochastic events are simulated as such, using pseudo random numbers generated on the fly by [Mersenne Twister](https://en.wikipedia.org/wiki/Mersenne_Twister).

If a stochastic event occurs at a constant rate, its time of occurence in generated to follow an [exponential distribution](https://en.wikipedia.org/wiki/Exponential_distribution), using the standard method:

	time = -log(random()) / rate

where `random()` returns a random number uniformly distributed in ]0,1] (zero is excluded). This applies [inverse transform sampling](https://en.wikipedia.org/wiki/Inverse_transform_sampling) to generate exponential variates. The resulting variable `time` is exponentially distributed with expectancy `1/rate`.


# Bell's Law and Kramers' Reaction Rate Theory

The detachment of a molecular link is promoted by force experienced by the link. The dependence is assumed to be exponential and the detachment rate reads:

	off_rate = unbinding_rate * exp( force / unbinding_force )
 
where `unbinding_rate` and `unbinding_force` are constant parameters associated with the bound state. In cytosim, these parameters are specified in the definitions of `Hands`. The `force` is the norm of the force vector calculated by `cytosim`, at every time step. Thus, `off_rate` varies with time. 

The same relationship can be expressed as:

	off_rate = unbinding_rate * exp( force * molecular_scale / ( kB * temperature ) )

Where `molecular_scale` is a distance that is supposed to reflect some dimension of the link, and `kB` is [Boltzman's constant](https://en.wikipedia.org/wiki/Boltzmann_constant).
This law is sometimes called Bell's law:

> [Models for the specific adhesion of cells to cells](http://dx.doi.org/10.1126/science.347575)  
> Bell, G. I. - Science, 200(4342), 618–627 - 1978

It comes as a particular limit in the theory of Hendrik Kramers:

>  [Brownian motion in a field of force and the diffusion model of chemical reactions ](https://doi.org/10.1016/S0031-8914(40)90098-2)  
>  H.A. Kramers - Physica VII, no 4, pp284-304 - 1940

For more information, see:

> [The load dependence of rate constants](https://doi.org/10.1063/1.2920475)  
> Sam Walcott - J. Chem Phys 128, 215101 - 2008

# Time-varying Rates

Using Kramers rate theory implies that the rate of the event is varying in time.  
The Gillespie approach needs to be modified, and we follow the procedure described in:
 
>  [A Dynamical Monte Carlo Algorithm for Master Equations with Time-Dependent Transition Rates](http://dx.doi.org/10.1007/BF02765541)  
>  A. Prados et al. Journal of Statistical Physics, Vol. 89, Nos. 3/4 - 1997  
 
In practice, a normalized time `esp` is first generated,
again using a random number uniformly distributed in [0,1] provided by `random()`.
At each time step, `esp` is reduced as a function of the value of the rate during the interval.
The associated event is performed if `esp` becomes negative, which is when the associated time
crosses the 'present' into the past.
  
	time = 0;
	esp = -log(random());
	
	while ( time < max_time )
	{
		time = time + time_step;
		esp = esp - time_step * rate(time);
		while ( esp < 0 )
		{
		    do_event();
		    esp = esp - log(random())
		}
	}
 
This code must be adapted depending on the circumstances. In this example, the event can be performed multiple times in the same time step, but this would not be done for detachment and other events that can only occur once. 

13.11.2019
