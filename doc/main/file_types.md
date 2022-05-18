# File types
 
Cytosim's ecosystem uses these file extensions:
 
Extension | Type             | Typical usage   
----------|------------------|--------------------------------------------------------
`*.cym`   | text file, input | configuration file
`*.cmo`   | text or binary   | input/output trajectory files for `sim`
`*.cyp`   | text file, input | configuration file for `play` with display parameters


# Files associated with `sim`
 
`sim` only has one input file: the config file (by default *config.cym*), and usually produces 3 output:
 
File             | Type           | Content  
-----------------|----------------|--------------------------------------------------
`properties.cmo` | text           | properties of the objects
`objects.cmo`    | binary or text | positions of the objects at different time points
`messages.cmo`   | text           | information such as the execution time 
 
The file `objects.cmo` is called the trajectory file.
 
# Files associated with `play`
 
`play` in the normal replay mode reads:
 
 File             | Type           | Content  
------------------|----------------|--------------------------------------------------
`properties.cmo`  | text           | properties of the objects
`objects.cmo`     | binary or text | positions of the objects at different time points
`style.cyp`       | text           | display parameters

However, `play live` reads the configuration file and usually does not write any file,
except to save images or export simulation snapshot (a trajectory file containing only one time point), see menu -> export.
 

# Files associated with `report`

`report` reads `properties.cmo` and `objects.cmo`, and sends output to the terminal.

