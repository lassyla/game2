@echo off

.PHONY : all

REM n.b. the '-y' sets autoexec scripts to 'on' so that driver expressions will work
UNAME_S:=$(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	set BLENDER=/Applications/Blender/blender.app/Contents/MacOS/blender -y
else ifeq ($(UNAME_S),Linux)
	set BLENDER=../../blender-2.90.0-linux64/blender -y
else
	set BLENDER=blender
endif

set EXPORT_MESHES=export-meshes.py
set EXPORT_SCENE=export-scene.py

set DIST=../dist

all : \
	$(DIST)/hexapod.pnct \
	$(DIST)/hexapod.scene \


$(DIST)/hexapod.scene : hexapod.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':Main '$@'

$(DIST)/hexapod.pnct : hexapod.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':Main '$@'
