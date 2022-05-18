# Physical units in Cytosim
 
 Cytosim is not aware of units, and it is the user's responsability to provide coherent values. The parameters could in principle be specified in any system of units, but the same system must be used consistently for all values! However, for convenience, Cytosim provides default values for some parameters, for example:

 Parameter   | Default value    |
 ------------|-------------------
 viscosity   | 1      pN.s/um^2
 kT          | 0.0042 pN.um

 It is therefore strongly advised to convert all parameter values to Cytosim's preferred system of units:

 Quantity    | Unit        | Symbol | Value                           |
 ------------|-------------|--------|----------------------------------
 Distance    | micrometer  | um     | 10^(-6) = 0.000001 meter
 Force       | picoNewton  | pN     | 10^(-12) = 0.000000000001 Newton
 Time        | second      | s      | 1/60 minute
 Angle       | radian      | rad    | PI radian = 180 degrees


 The most common physical quantities have these units:

 Parameter          | Units     |
 -------------------|------------
 duration           |  s
 length or range    |  um
 force              |  pN
 rate               |  1/s
 torque             |  pN.um
 speed              |  um/s
 diffusion constant |  um^2/s
 stiffness          |  pN/um
 angular stiffness  |  pN.um/rad
 energy             |  pN.um
 bending elasticity |  pN.um^2
 viscosity          |  pN.s/um^2
 
The number of objects are usually specified directly and if you know a concentration,
you will need to multiply by the volume of the cell. The binding and unbinding rates are
specified as molecular rate (1/s) and knowing the equilibrium constant of the reaction is
usually not sufficient to deduce both rates, although knowing the equilibrium constant should
help you to constrain the ratio of the binding to the unbinding rate.

