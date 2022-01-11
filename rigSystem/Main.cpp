#define _CRT_SECURE_NO_WARNINGS

#undef NT_PLUGIN
#define MNoVersionString
#include <maya/MFnPlugin.h>
#undef MNoVersionString
#define NT_PLUGIN

#ifdef WIN32
	#define _WINSOCKAPI_
	#define NOMINMAX
	#include <windows.h>
#endif

#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MProfiler.h>

#include "rigSystemControlNode.h"

static const char* VERSION = "4.0.20220110";

MStatus initializePlugin(MObject obj)
{
	MFnPlugin plugin(obj, "Perry Leijten", VERSION, "Any");


	plugin.registerNode("RigSystemControl", RigSystemControlNode::id,&RigSystemControlNode::creator,&RigSystemControlNode::initialize,MPxNode::kLocatorNode, &RigSystemControlNode::drawDbClassification);
	MHWRender::MDrawRegistry::registerDrawOverrideCreator(RigSystemControlNode::drawDbClassification, RigSystemControlNode::drawRegistrantId, RigSystemControlDrawOverride::Creator);

	MSelectionMask::registerSelectionType(RigSystemControlNode ::kSelectionMaskName, 3);

	MString cmd;
	cmd.format("selectType -byName \"^1s\" 1", RigSystemControlNode::kSelectionMaskName);
	MGlobal::executeCommand(cmd);
	
	MString cmd1 = "cacheEvaluator -newFilter \"nodeTypes\" -newFilterParam \"types=+RigSystemControl\" -newAction \"enableEvaluationCache\";";
	MGlobal::executeCommand(cmd1);

	gProfilerCategory = MProfiler::addCategory("RSControl", "Time taken by RSControlNode");

	return MS::kSuccess;
};

MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(RigSystemControlNode::drawDbClassification, RigSystemControlNode::drawRegistrantId);
	plugin.deregisterNode(RigSystemControlNode::id);

	MSelectionMask::deregisterSelectionType(RigSystemControlNode ::kSelectionMaskName);
	
	return MS::kSuccess;
};

