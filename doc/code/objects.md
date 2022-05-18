# Simulation Objects

### Simul

 The command `set simul` will define the global parameters.
 There can only be one `simul` object, and it is automatically created,
 so you do not need to call 'new simul'.

 Simul - SimulProp - @ref SimulPar
 
### Surfaces & Volumes
 
 These objects are immobile:

   Class       | Parameters       | Property     | Options       
 --------------|------------------|--------------|-------------------------
 Space         | @ref SpacePar    | SpaceProp    | @ref SpaceGroup
 Field         | @ref FieldPar    | FieldProp    | @ref FieldSet::newObjects
 Event         | @ref EventPar    | EventProp    | (unfinished)
 
 
### Mecables
 
  `Mecables` can move or deform. There are 4 basic classes:

   Class       | Parameters       | Property     | Options    
 --------------|------------------|--------------|---------------------------
 Fiber         | @ref FiberPar    | FiberProp    | @ref FiberGroup
 Bead          | @ref SolidPar    | SolidProp    | @ref BeadSet::newObjects
 Solid         | @ref SolidPar    | SolidProp    | @ref SolidSet::newObjects
 Sphere        | @ref SpherePar   | SphereProp   | @ref SphereSet::newObjects

### Fibers
 @ref FiberGroup

  `activity`   | Class               | Parameters                | Property       
 --------------|---------------------|---------------------------|----------------------
 `none`        | Fiber               | @ref FiberPar             | FiberProp
 `grow`        | GrowingFiber        | @ref GrowingFiberPar      | GrowingFiberProp
 `dynamic`     | DynamicFiber        | @ref DynamicFiberPar      | DynamicFiberProp
 `classic`     | ClassicFiber        | @ref ClassicFiberPar      | ClassicFiberProp
 `treadmill`   | TreadmillingFiber   | @ref TreadmillingFiberPar | TreadmillingFiberProp
 
#### Hands
 
 A `Hand` is an object that can bind to fiber, but it can only be used
 as a subpart of `Single` or `Couple`.

 @ref HandGroup
 
  `activity`   | Class         | Parameters         | Property     |
 --------------|---------------|--------------------|---------------
 `bind`        | Hand          | @ref HandPar       | HandProp
 `move`        | Motor         | @ref MotorPar      | MotorProp
 `nucleate`    | Nucleator     | @ref NucleatorPar  | NucleatorProp
 `slide`       | Slider        | @ref SliderPar     | SliderProp
 `track`       | Tracker       | @ref TrackerPar    | TrackerProp
 `rescue`      | Rescuer       | @ref RescuerPar    | RescuerProp
 `regulate`*   | Regulator     | @ref RegulatorPar  | RegulatorProp
 `cut`         | Cutter        | @ref CutterPar     | CutterProp
 `chew`        | Chewer        | @ref ChewerPar     | ChewerProp
 `mighty`      | Mighty        | @ref MightyPar     | MightyProp
 `act`         | Actor         | @ref ActorPar      | ActorProp

### Digital Hands

  `activity`   | Class         | Parameters         | Property     |
 --------------|---------------|--------------------|---------------
 `digit`       | Digit         | @ref DigitPar      | DigitProp
 `walk`        | Walker        | @ref WalkerPar     | WalkerProp
 `kinesin`*    | Kinesin       | @ref KinesinPar    | KinesinProp
 `dynein`*     | Dynein        | @ref DyneinPar     | DyneinProp
 `myosin`*     | Myosin        | @ref MyosinPar     | MyosinProp
 
 `*` Unfinished classes.


### Singles
 
 A `Single` contains one `Hand`, and can have different `activity`:

 @ref SingleGroup
 
 `activity`     | Classes           | Parameters      | Property   |
 ---------------|-------------------|-----------------|-------------
 `diffuse`      | Single            | @ref SinglePar  | SingleProp
 `fixed`        | Picket PicketLong | @ref SinglePar  | SingleProp
 not applicable | Wrist  WristLong  | @ref SinglePar  | SingleProp
 
### Couples
 
 A `Couple` contains two `Hand`, and can have different `activity`:

 @ref CoupleGroup
 
 `activity`    | Classes                 | Parameters           | Property     |
 --------------|-------------------------|----------------------|---------------
 `diffuse`     | Couple CoupleLong       | @ref CouplePar       | CoupleProp
 `crosslink`   | Crosslink CrosslinkLong | @ref CrosslinkPar    | CrosslinkProp
 `bridge`      | Bridge                  | @ref BridgePar       | BridgeProp
 `duo`         | Duo  DuoLong            | @ref DuoPar          | DuoProp
 `slide`       | Shackle ShackleLong     | @ref ShacklePar      | ShackleProp
 `fork`        | Fork                    | @ref ForkPar         | ForkProp

 
### Organizers
 
 The `Organizers` describe composite objects build from multiple Mecables:

 Class         | Parameters       | Property     |
 --------------|------------------|---------------
 Aster         | @ref AsterPar    | AsterProp
 Fake          | @ref FakePar     | FakeProp
 Bundle        | @ref BundlePar   | BundleProp
 Nucleus       | @ref NucleusPar  | NucleusProp
 


