all : \
    ../dist/pond.pnct \
    ../dist/pond.scene \

../dist/pond.scene : pond.blend export-scene.py
    "C:\Program Files\Blender Foundation\Blender 2.90\blender.exe" --background --python export-scene.py -- "pond.blend:Main" "../dist/pond.scene"

../dist/pond.pnct : pond.blend export-meshes.py
    "C:\Program Files\Blender Foundation\Blender 2.90\blender.exe" --background --python export-meshes.py -- "pond.blend:Main" "../dist/pond.pnct"