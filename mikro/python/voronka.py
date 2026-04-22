import pysplishsplash as sph
import pysplishsplash.Utilities.SceneLoaderStructs as Scenes
import math

def main():
    base = sph.Exec.SimulatorBase()
    base.init(useGui=True, sceneFile=sph.Extras.Scenes.Empty)
    gui = sph.GUI.Simulator_GUI_imgui(base)
    base.setGui(gui)
    base.setVec3ValueReal(base.CAMERA_POSITION, [0, 4, 4])
    base.setVec3ValueReal(base.CAMERA_LOOKAT, [0, 0.2, 0])
    
    scene = sph.Exec.SceneConfiguration.getCurrent().getScene()
    scene.particleRadius = 0.022
    
    scene.boundaryModels.append(Scenes.BoundaryData(
        meshFile="../models/UnitBox.obj", 
        translation=[0., 0.2, 0.],
        scale=[2.5, 0.6, 2.5],
        color=[0.1, 0.4, 0.5, 1.0], 
        isWall=True, 
        mapInvert=True,
        mapResolution=[35, 35, 35],
        isDynamic=False
    ))
    radius = 0.8
    speed = 1.5
    for angle in range(0, 360, 45):
        rad = math.radians(angle)
        x = radius * math.cos(rad)
        z = radius * math.sin(rad)
        vx = -math.sin(rad) * speed
        vz = math.cos(rad) * speed
        
        scene.fluidBlocks.append(Scenes.FluidBlock(
            id='Water', 
            boxMin=[x-0.25, -0.15, z-0.25], 
            boxMax=[x+0.25, 0.1, z+0.25], 
            mode=0, 
            initialVelocity=[vx, 0.0, vz]
        ))
    
    base.run()

if __name__ == "__main__":
    main()
