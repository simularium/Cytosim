[[length=random.uniform(5, 15)]]% [[length]]

set simul system
{
    time_step = 0.01
    viscosity = 0.01
}

set space cell
{
    shape = sphere
}

new cell
{
    radius = 10
}

set fiber microtubule
{
    rigidity = 30
    segmentation = 1
    confine = inside, 100

    display = { line_width=3; }
}

set solid core
{
    display = ( style=3 )
}

set aster centrosome
{
    stiffness = 1000, 500
}

new centrosome
{
    solid = core
    radius = 0.5
    point0 = center, 0.5
    fibers = 32, microtubule, ( plus_end=grow; length = [[length]]; )
    position = 0 0
}

run system
{
    nb_steps = 5000
    nb_frames = 10
}

report aster aster.txt

