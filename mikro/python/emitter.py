import pysplishsplash as sph
import pysplishsplash.Utilities.SceneLoaderStructs as Scenes
import math

def main():
    base = sph.Exec.SimulatorBase()
    base.init(useGui=True, sceneFile=sph.Extras.Scenes.Empty)
    gui = sph.GUI.Simulator_GUI_imgui(base)
    base.setGui(gui)
    base.setVec3ValueReal(base.CAMERA_POSITION, [0, 3.5, 5.0])
    base.setVec3ValueReal(base.CAMERA_LOOKAT, [0, 0.2, 0])
    scene = sph.Exec.SceneConfiguration.getCurrent().getScene()

    scene.particleRadius = 0.025
    
    scene.boundaryModels.append(Scenes.BoundaryData(
        meshFile="../models/UnitBox.obj", 
        translation=[0., 0.2, 0.],
        scale=[2.5, 0.6, 2.5],
        color=[0.1, 0.4, 0.5, 1.0], 
        isWall=True, 
        mapInvert=True,
        mapResolution=[25, 25, 25]
    ))
    
    r = 1.1
    #scene.emitters.append(Scenes.EmitterData(id='E1', x=[r, 0.3, 0], width=3, height=3, axis=[0, 0, -1], angle=1.0, velocity=1.5))
    scene.emitters.append(Scenes.EmitterData(id='E2', x=[-r, 0.3, 0], width=3, height=3, axis=[0, 0, 1], angle=0.2, velocity=1.5))
    scene.emitters.append(Scenes.EmitterData(id='E3', x=[0, 0.3, r], width=3, height=3, axis=[1, 0, 0], angle=0.2, velocity=1.5))
    scene.emitters.append(Scenes.EmitterData(id='E4', x=[0, 0.3, -r], width=3, height=3, axis=[1, 0, 0], angle=0.2, velocity=1.5))

    base.run()

if __name__ == "__main__":
    main()

