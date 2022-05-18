
# Motion-induced motor detachment

Affects class Motor and Mighty. Implemented on 03/03/2018, FJN

To enable this feature for Motor, set:

	#define NEW_UNBINDING_DENSITY 1
    
### Description

For a motor that moves, the unbinding rate can depend on the motion of this motor.
In cytosim, the rate is normally set per unit time, and it can also depend on force, as:

	unbinding_probability1 = time_step * unbinding_rate * exp(force/unbinding_force)

(One can set `unbinding_force = inf` to have a constant detachment rate)
Another term was added to make detachment proportional to the distance travelled by the motor:

	unbinding_probability2 = unbinding_density * linear_displacement

Where `linear_displacement` is the movement of the motor along the filament during the last `time_step`. This way a motor which is moving has a rate increased by:

	unbinding_density * unloaded_speed

In contrast, a motor which has reached the end of a filament has the first term only, since its displacement is zero. Hence with this new model, the unbinding rate of a motor dwelling at the end can be lower than that of amotor moving on the side. 
In addition, this also affects motors which are stalled by force, since as they do not move, detachment is set by the first term only.
The effect is most noticable if `unbinding_rate` is small. Specifically:

	unbinding_rate << unbinding_density * unloaded_speed

gives an elementary catch-bond behavior.

Please run `cym/motor_race.cym` to check the effect of different unbinding models.

