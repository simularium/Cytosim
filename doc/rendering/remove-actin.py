# Francois Nedelec, 05.02.2018 -- 06.02.2018

import bpy

# Shortcuts
bld = bpy.data
scene = bpy.context.scene

def remove_objects(name):
    for obj in scene.objects:
        if obj.name.startswith(name):
            bld.objects.remove(obj)

remove_objects("actin");
remove_objects("link");
