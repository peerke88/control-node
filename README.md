# control-node
simple MPxLocator that holds multiple shapes for use in rigging

This is an example of a node that I have been using for over 6 years now, currently releasing it as its still not fast enough to completely discard the original NurbsCurve controls in a rig.

main reason to release this is to give an example on how the code works and expand knowledge as I raised the question here:

https://forums.autodesk.com/t5/maya-programming/custom-locators-for-rig-controls-mpxdrawoverride-being-slow/td-p/10866744

## Prerequisites

```
 - cmake 3.6+
 - python 3
 - Maya 2022
 - visual studio 16
 - rapidJson
```


## launch

in order to build the package all previously mentioned elements need to be installed 
in order for rapidJson to work it can be placed in the rigSystem folder, or cmake needs to be modified in order to include the correct path to rapidJson

python 3 is necessary to convert the json file to a binary resource that is loaded as *resources.h* file. 

building the plugin is as simple as launching the *Build_maya_plugins.bat* file

## current status


- the locator is created using the MPxLocator and MPxDrawOverride with MUIDrawManager, this is all derived from the *footPrintNode.h* included in the maya devkit examples

- vp1 is completely removed as there is no need for it.

- added profiling tags into the MPxDrawOverride functions

- in the comp folder the latest compiled version of the plugin can be found for maya 2022


## quick test python script:

```python
from maya import cmds
fname = r"#~~~~\control-node\comp\Maya2022\plug-ins\rigSystem.mll"
cmds.loadPlugin(fname)

for i in range(100):
    cmds.createNode("RigSystemControl")

```