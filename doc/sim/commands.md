# Cytosim commands

The normal syntax of a command is:
 
     COMMAND ARGUMENT1 ARGUMENT2
     {
         PARAMETER = VALUE1, VALUE2, ...;
         ...
     }

where `COMMAND` is the name of the command, with `ARGUMENTS` separated by space, and followed by a curly braced `{}` block of parameter specifications.

Some commands offer a short syntax:
 
     COMMAND ARGUMENT1 ARGUMENT2
 
### Essential commands
 
 Command        | Description 
 ---------------|-----------------------------------------------------
 `set`          | Create a new Property, and set its parameter values
 `new`          | Create objects of a certain Property
 `run`          | Simulate the system to advance in time
 
### Useful commands
 
 Command        | Description
 ---------------|---------------------------------------------------------
 `change`       | Change parameter values in an existing Property
 `delete`       | Delete objects from the simulation
 `read`         | Read another file and excutes the commands it contains
 `import`       | Import objects from a trajectory file
 `export`       | Export all simulated objects to a file
 `report`       | generate file or text with formatted information
 `repeat`       | Execute code a number of times

### Other commands
 
 Command        | Effect 
 ---------------|---------------------------------------------------------
 `mark`         | Mark objects
 `cut`          | Cut fibers along a plane
 `for`          | Execute code a number of times (disabled)
 `end`          | Terminates simulation
 `call`         | Call a custom function


# `set`

 Create a new Property, which is a set of parameters associated with a class.
 
     set CLASS NAME
     {
       PARAMETER = VALUE
       ...
     }
 
 `CLASS` should be one of the predefined object class [objects](objects.md).
`NAME` is a string you may choose, starting with a letter, followed by alphanumeric characters. The underscore character is also allowed, as in `fiber_3`. The list of parameter depends on the class.
 
You may use `set` to change values of an existing Property:
 
     set NAME
     {
       PARAMETER = VALUE;
       ...
     }
 
 and equivalently:
 
     set NAME { PARAMETER = VALUE; }
 
 or the shorter version:
 
     set NAME PARAMETER { VALUE }
 

# `new`

 The command `new` creates one or more objects with given specifications:
 
     new [MULTIPLICITY] NAME
     {
       position         = POSITION, [SPACE]
       direction        = DIRECTION
       orientation      = ROTATION, [ROTATION]
       mark             = INTEGER
       required         = INTEGER
     }
 
 The NAME should have been defined previously with `set`.

 The accepted parameters are:
 
 Parameter        | Type      | Default | Description                          |
 -----------------|-----------|---------|---------------------------------------
 MULTIPLICITY     | INTEGER   |   1     | number of objects.
 `position`       | POSITION  | random  | initial position of the object.
 `orientation`    | ROTATION  | random  | a rotation specified with respect to the object's center of gravity.
 `orientation[1]` | ROTATION  | none    | a rotation specified around the origin.
 `direction`      | DIRECTION | random  | specifies the direction of a fiber.
 `mark`           | INTEGER   |   0     | specifies a mark to be given to all objects created.
 `required`       | INTEGER   |   0     | minimum number of objects that should be created.
 

 Note that `position` only applies to movable objects, and `orientation` will have an effect only on objects that can be rotated. In addition, `position[1]` and `orientation[1]` are relevant only if `(MULTIPLICITY > 1)`, and do not apply to the first object.
 
 Short syntax:
 
     new [MULTIPLICITY] NAME ( POSITION )
 
 Shorter syntax:
 
     new [MULTIPLICITY] NAME
 
# `run`

 To perform simulation steps for a *Simul* object called `NAME`, use:
 
     run POSITIVE_INTEGER NAME
     {
        duration   = POSITIVE_REAL
        solve      = SOLVE_MODE
        event      = RATE, ( CODE )
        nb_frames  = INTEGER, ( CODE )
        prune      = BOOLEAN
        flux_speed = NEGATIVE_REAL
     }
 
 or
 
     run NAME
     {
        nb_steps   = POSITIVE_INTEGER
        ...
     }

 or, without specifying the Name of the Simul:
 
     run [POSITIVE_INTEGER] all simul
     {
        ...
     }

 
 The optional parameters are:
 
 Parameter    | Default | Description                                          |
 -------------|---------|-------------------------------------------------------
 `nb_steps`   |  `1`    | number of simulation steps
 `duration`   |  `- `   | when specified, `nb_steps` is set to `ceil(duration/time_step)`
 `solve`      |  `on`   | Define the type of method used for the mechanics
 `event`      |  `none` | custom code executed stochastically with prescribed rate
 `nb_frames`  |  `0`    | number of states written to trajectory file
 `prune`      |  `true` | Print only parameters that are different from default
 
 
 The parameter `solve` can be used to select alternative mechanical engines.
 The monte-carlo part of the simulation is always done, including
 fiber assembly dynamics, binding/unbinding and diffusion of molecules.
 
 `solve`      | Result                                                         |
 -------------|-----------------------------------------------------------------
 `off`        | Objects are immobile.
 `on`         | The mechanics is solved fully (default).
 `auto`       | Same as 'on' but preconditionning method is set automatically.
 `horizontal` | Objects can move in the X-direction. The mechanics is solved partly.
 `flux`       | Fibers are translated at `flux_speed` according to their orientation.
 
 If set, `event` defines an event occuring at a rate specified by the positive real `RATE`.
 The action is defined by CODE, a string enclosed with parenthesis containing cytosim commands.
 This code will be executed at stochastic times with the specified rate.
 
 Example:

     event = 10, ( new actin { position=(rectangle 1 6); length=0.1; } )
 
 Calling `run` will not output the initial state, but this can be done with a separate command:
 
     export objects objects.cmo { append = 0 }
 
     run 1000 system
     {
        nb_frames = 10
     }


---


# `change`

 Change the value of one (or more) parameters for property `NAME`.

     change NAME
     {
       PARAMETER = VALUE
       ...
     }
 
 Short syntax:
 
    change NAME { PARAMETER = VALUE }

The NAME should have been defined previously with the command `set`.
It is also possible to change all properties of a particular class:
 
     change all CLASS
     {
       PARAMETER = VALUE
       ...
     }

Examples:
 
    change system { viscosity = 0.5; }
    change system display { back_color=red; }
    change actin { rigidity = 1; }
    change microtubule { rigidity = 20 }
    change all fiber { confine = inside, 10; }
    change all fiber display { color = white; }

 

# `delete`

 Delete objects:

     delete [MULTIPLICITY] NAME
     {
        mark      = INTEGER
        position  = [inside|outside], SPACE
        state     = [0|1], [0|1]
     }
 
 NAME can be '*', and the parameters (mark, position, state) are all optional.
 All specified conditions must be fulfilled (this is a logical AND).
 The parameter `state` refers to bound/unbound state of Hands for Single and Couple,
 and to dynamic state for Fibers:
 - for Single, `state[0]` refers to the Hand: 0=free, 1=attached.
 - for Couple, `state[0]` refers to the first Hand: 0=free, 1=attached,
           and `state[1]` refers to the second Hand: 0=free, 1=attached.
 - for Fibers, `state[0]` refers to the Dynanic state of the PLUS end,
           and `state[1]` refers to the Dynanic state of the MINUS end.
 .
 
 To delete all objects of specified NAME:
 
     delete all NAME
 
 To delete at most CNT objects of class NAME:
 
     delete CNT NAME
 
 To delete all objects with a specified mark:
 
     delete all NAME
     {
        mark = INTEGER
     }
 
 To delete all objects within a Space:
 
     delete NAME
     {
        position = inside, SPACE
     }
 
 The SPACE must be the name of an existing Space.
 Only 'inside' and 'outside' are valid specifications.

 To delete all Couples called NAME that are not bound:
 
     delete all NAME { state1 = 0; state2 = 0; }
 
 To delete all Couples called NAME that are not crosslinking, use two calls:
 
     delete all NAME { state1 = 0; }
     delete all NAME { state2 = 0; }
 
# `read`

 Read and execute another config file.
 
     read FILE_NAME
     {
       required = BOOLEAN
     }
 
 By default, `required = 1`, and execution will terminate if the file is not found.
 If `required=0`, the file will be executed if it is found, but execution will continue
 in any case.

# `import`

 Import a simulation snapshot from a trajectory file
 
    import WHAT FILE_NAME
    {
        append = BOOLEAN
        frame = INTEGER
    }
 
 The frame to be imported can be specified as an option: `frame=INTEGER`:
 
     import all my_file.cmo { frame = 10 }
 
 By default, this will replace the simulation state by the one loaded from file.
 To add the file objects to the simulation without deleting any of the current 
 object, you should specify `append = 1`:
 
     import all my_file.cmo { append = 1 }
 
 Finally instead of importing all the objects from the file, one can restrict
 the import to a desired class:
 
     import fiber my_file.cmo { append = 1 }
 
 Note that the simulation time will be changed to the one specified in the file,
 but this behavior can be changed by specifying the time:
 
     change system { time = 0 }
 
 ...assuming that the simul is called `system`.
 
# `export`

 Export state to file. The general syntax is:
 
     export WHAT FILE_NAME
     {
       append = BOOLEAN
       binary = BOOLEAN
     }
 
 WHAT must be ``objects`` of ``properties``, and by default, both `binary` 
 and `append` are `true`. If `*` is specified instead of a file name,
 the current trajectory file will be used.
 
 Short syntax:
 
     export objects FILE_NAME
 
 Examples:
 
     export all sim_objects.cmo { append=0 }
     export properties properties.txt
 
 Attention: this command is disabled for `play`.
 
# `report`

 Export formatted data to file. The general syntax is:
 
     report WHAT FILE_NAME
     {
       append = BOOLEAN
     }
 
 Short syntax:
 
     report WHAT FILE_NAME
 
 WHAT should be a valid argument to `report`:
 @copydetails Simul::report
 
 If `*` is specified instead of a file name, the report is sent to the standard output.
 
 Examples:
 
     report parameters parameters.cmo { append=0 }
     report fiber:length fibers_length.txt
 
 Note that this command is disabled for `play`.
 
# `repeat`
 
 Repeat specified code.
 
     repeat INTEGER { CODE }


---

# `mark`


 Mark objects:
 
     mark [MULTIPLICITY] NAME
     {
       mark       = INTEGER
       position   = POSITION
     }
 
 or
 
     mark all NAME
     {
         mark       = INTEGER
         position   = POSITION
     }

 NAME can be '*', and the parameter 'position' is optional.
 The syntax is the same as for command `delete`.
 
# `for` 

 Repeat specified code.
 
     for VAR=INTEGER:INTEGER { CODE }
 
 The two integers are the first and last iterations counters.
 Any occurence of VAR is replaced before the code is executed.
 
 Example:
 
     for CNT=1:10 {
       new 10 filament { length = CNT }
     }
 
 NOTE: This code is a hack, and can be improved vastly!

# `cut`

 Cut all fibers that intersect a given plane.
 
     cut fiber NAME
     {
        plane = VECTOR, REAL
     }
 
     cut all fiber
     {
        plane = VECTOR, REAL
     }

 NAME can be '*' to cut all fibers.
 The plane is specified by a normal vector `n` (VECTOR) and a scalar `a` (REAL).
 The plane is defined by <em> n.pos + a = 0 </em>

# `call`

 Call custom function:
 
     call FUNCTION_NAME { OPTIONS }
 
 FUNCTION_NAME should be `equilibrate`, `custom0`, `custom1`, ... `custom9`.

 Note: The Simul::custom() functions need to be modified, to do something!


# `end` 

 Terminates execution
 
     end
