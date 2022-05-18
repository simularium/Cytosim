# Fibers

**UNFINISHED**

Fibers are cytosim's representation of cytoskeletal filaments, i.e. actin and microtubules. They are constituted of discrete segments, the length of those is the _segmentation_. The fibers are rigid, i.e. segments tend to be aligned, this is controlled by the _rigidity_. Fibers can be of fixed length or be dynamic ; this is described by their _activity_. They can be restricted to the inside of a volume, as prescribed by the keyword _confine_. They can also interact sterically with one another or with other objects, as set by _steric_.


# Parameters of a fiber
### segmentation
_segmentation_ (unit : length) is the length of segments making the filament. The shorter the segmentation, the more precise the simulation but the more time it takes.

### rigidity 
_rigidity_ (unit : energy x length) is the rigidity of the filament. Typically 20 pN µm^2 for a microtubule.

### activity
*classic* : Classic model of microtubule dynamics, with a catastrophe rate, rescue rate, growth rate, shrinkage rate. Catastrophe rate can be force-dependent.  
*treadmill* : a simple model of fiber dynamics that can grow or shrink at both ends, in a force-dependent manner.  
*dynamic* : a more advanced model of microtubule dynamics including the hydrolysis of a GTP cap.  
*grow* : a simple growing filament  

### display
How the filament is displayed.

*size* : the thickness of the line  
color : the color of the filament

# Examples

### Microtubules of fixed length

```
set fiber microtubule
{
    rigidity = 20  
    segmentation = 0.5  
    confine = inside, 200  
    display = ( line_width=8 )  
}  

new 3 microtubule
{  
    length = 3  
}  
```
