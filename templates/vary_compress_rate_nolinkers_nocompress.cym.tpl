% Fiber under compression to measure buckling force


% how fast the end gets compressed (µm/s) 150 nm / 10 µs
[[compression_velocity = [0.15, 0.47434165, 1.5, 4.73413649, 15, 47.4341649, 150]]]
% fast: 1.5e2, 1.5e3, 1.5e4
% slow: 0.0015, 0.015, 0.15, 1.5, 15

% the time step (s)
[[timestep = 5e-07]]

% how frequently the compression is applied (time steps)
[[compression_interval = 10]]

% frequency you save frames (s)
[[save_time_interval = 5e-07]]

% total fiber length
[[fiber_length = 0.5]]

% ratio of total length that gets compressed
[[compression_ratio = 0.3]]


% total compression amount
[[compression_distance = fiber_length*compression_ratio]]

% total simulation time (s)
[[total_time = round(compression_distance/compression_velocity, 7)]]

% compression_velocity:  [[compression_velocity]]
% compression_distance:  [[compression_distance]]
% total_time: [[total_time]]
% [[round(total_time/timestep/compression_interval)]] repeats
% [[round(timestep/save_time_interval)]] frames saved per repeat
% translation = [[round(compression_velocity*timestep*compression_interval,7)]]

set simul system
{
    time_step = [[timestep]]
    viscosity = 0.01
}

set space cell
{
    shape = sphere
}

new cell
{
    radius = 2
}

set fiber filament
{
    rigidity = 0.041
    segmentation = 0.001
    display = ( point=6,1; line=10,2; )
}

set hand binder
{
    % 10 events per second, 0.05 um binding radius

    binding = 10, 0.005

    % 0 events per second, infinite unbinding force

    unbinding = 0, inf
    bind_also_end = 1

    display = ( width=3; size=12; color=green )
}

set single linker
{
    hand = binder
    activity = fixed
    stiffness = 10000000
}


new filament
{
    length = [[fiber_length]]
    position = 0 0 0
    orientation = 1 0 0
    % attach1 = linker, 0, minus_end
    % attach2 = linker, 0, plus_end

}


% number of intervals for compression
repeat [[round(total_time/timestep/compression_interval)]]
{

% this means you run one step and let it relax 9 time steps
% and show it for 2 frames
run [[compression_interval]] system { nb_frames = [[round(timestep/save_time_interval)]] }

report fiber:points fiber_points.txt
report single:position singles.txt


}
