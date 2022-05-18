% Fiber under compression to measure buckling force
% F. Nedelec, 21.02.2018

set simul system
{
    time_step = 0.001
    viscosity = 0.01
}

set space cell
{
    shape = sphere
}

new cell
{
    radius = 5
}

set fiber filament
{
    rigidity = 10
    segmentation = [[nseg=range(2,120)]][[round(12.0/nseg,4)]]
}

set hand binder
{
    binding = 10, 0.05
    unbinding = 0, inf
	bind_also_end = 1
}

set single link
{
    hand = binder
    activity = fixed
    stiffness = 1000
}

new filament
{
    length = 10
    position = 0 0 0
    orientation = 1 0 0
}

new link
{
    position = -4.5 0 0 
    attach = fiber1, 0
}

new link
{
    position = +4.5 0 0 
    attach = fiber1, 10
}

run 10000 system
{
    nb_frames = 10
}

report single:force force.txt { verbose=0; }
report fiber:tension tension.txt { verbose=0; plane = 1 0 0, 0; }
