import bpy





# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty, CollectionProperty
from bpy.types import Operator

def get_cam_keyframes(act, stream):
    a = bpy.data.actions[act]
    a.fcurves.update()
    i = 0
    for fcu in a.fcurves:
        fcu.update()
        stream.write(str(i) + "\n")
        i = i+1
        for keyframe in fcu.keyframe_points:
            stream.write(str(keyframe.co[0]) + "," + str(keyframe.co[1]) + "\n")

def write_some_data(context, filepath, value):
    print("running write_some_data...")
    if value == "":
        print("Choose a camera")
        return {'FINISHED'}
    print(value)
    f = open(filepath, 'w', encoding='utf-8')
    get_cam_keyframes(value, f)
    f.close()

    return {'FINISHED'}

def get_cams(scene, context):
    items = [("", "Select an action", "None")]
    for act in bpy.data.actions:
        print(act.name)
        items.append((act.name, act.name, "Action"))
    return items


class ExportSomeData(Operator, ExportHelper):
    """This appears in the tooltip of the operator and in the generated docs"""
    bl_idname = "export_test.some_data"  # important since its how bpy.ops.import_test.some_data is constructed
    bl_label = "Export Camera motion"

    # ExportHelper mixin class uses this
    filename_ext = ".txt"

    filter_glob: StringProperty(
        default="*.txt",
        options={'HIDDEN'},
        maxlen=255,  # Max internal buffer length, longer would be clamped.
    )

    # List of operator properties, the attributes will be assigned
    # to the class instance from the operator settings before calling.

    action: EnumProperty(
        name="Animations",
        description="Choose the animation curve",
        items=get_cams
    )
    

    def execute(self, context):
        return write_some_data(context, self.filepath, self.action)


# Only needed if you want to add into a dynamic menu
def menu_func_export(self, context):
    
    self.layout.operator(ExportSomeData.bl_idname, text="Export Camera motion")


def register():
    bpy.utils.register_class(ExportSomeData)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportSomeData)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()

    # test call
    #bpy.ops.export_test.some_data('INVOKE_DEFAULT')
