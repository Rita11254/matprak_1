import pysplishsplash as sph
import pysplishsplash.Utilities.SceneLoaderStructs as Scenes

def main():
    base = sph.Exec.SimulatorBase()
    base.init(useGui=True, sceneFile=sph.Extras.Scenes.Empty)
    gui = sph.GUI.Simulator_GUI_imgui(base)
    base.setGui(gui)
    base.setVec3ValueReal(base.CAMERA_POSITION, [6.0, 1.0, 1.0]) 
    base.setVec3ValueReal(base.CAMERA_LOOKAT, [0.0, 0.2, 0.0]) 
    scene = sph.Exec.SceneConfiguration.getCurrent().getScene()
    
    scene.boundaryModels.append(Scenes.BoundaryData(
        meshFile="../models/UnitBox.obj", 
        translation=[0., 0., 0.],
        scale=[2.0, 2.0, 2.0],
        color=[0.1, 0.4, 0.5, 1.0], 
        isWall=True, 
        mapInvert=True,
        mapResolution=[25, 25, 25]
    ))
    
    scene.fluidBlocks.append(Scenes.FluidBlock(
        id='Fluid', 
        boxMin=[-1., -1.0, -1.], 
        boxMax=[1., -0.5 , 1.], 
        mode=0, 
        initialVelocity=[0.0, 0.0, 0.0]
    ))

    scene.fluidModels.append(Scenes.FluidData(
        id='Fluid', 
        samplesFile="../models/sphere.obj", 
        mode=0, 
        scale=[0.2, 0.2, 0.2],
        translation=[0.0, 0.8, 0.0],
        initialVelocity=[0.0, -1.2, 0.0]
    ))
    

    base.run()

if __name__ == "__main__":
    main()
