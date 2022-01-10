#include "rigSystemControlNode.h"

MTypeId RigSystemControlNode::id(0x00131741);

MObject RigSystemControlNode::offsetMatrix;
MObject RigSystemControlNode::controlList;
MObject RigSystemControlNode::size;
MObject RigSystemControlNode::lineThickness;
MObject RigSystemControlNode::fill;
MObject RigSystemControlNode::front;
MObject RigSystemControlNode::worldS;



const MString RigSystemControlNode::kSelectionMaskName = "RigSystemControlNodeSelection";
MString	RigSystemControlNode::drawDbClassification("drawdb/geometry/ControlNode");
MString	RigSystemControlNode::drawRegistrantId("ControlNodePlugin");


// --- MPxLocatorNode
MStatus RigSystemControlNode::preEvaluation( const MDGContext& context, const MEvaluationNode& evaluationNode){
	if (context.isNormal())	{
		MStatus status;
		if (evaluationNode.dirtyPlugExists(size, &status) && status){
			MHWRender::MRenderer::setGeometryDrawDirty(thisMObject());
		}
	}

	return MStatus::kSuccess;
}


// --- MPxDrawOverride
RigSystemControlDrawOverride::RigSystemControlDrawOverride(const MObject& obj):MHWRender::MPxDrawOverride(obj, NULL, false), fRigSystemControl(obj) {
	fModelEditorChangedCbId = MEventMessage::addEventCallback( "modelEditorChanged", OnModelEditorChanged, this);

	MStatus status;
	MFnDependencyNode node(obj, &status);
	fRigSystemControlNode = status ? dynamic_cast<RigSystemControlNode*>(node.userNode()) : NULL;
}


RigSystemControlDrawOverride::~RigSystemControlDrawOverride(){
	fRigSystemControlNode = NULL;

	if (fModelEditorChangedCbId != 0){
		MMessage::removeCallback(fModelEditorChangedCbId);
		fModelEditorChangedCbId = 0;
	}
}


void RigSystemControlDrawOverride::OnModelEditorChanged(void *clientData){
	RigSystemControlDrawOverride *ovr = static_cast<RigSystemControlDrawOverride*>(clientData);
	if (ovr && ovr->fRigSystemControlNode){
		MHWRender::MRenderer::setGeometryDrawDirty(ovr->fRigSystemControlNode->thisMObject());
	}
}


MBoundingBox RigSystemControlDrawOverride::boundingBox( const MDagPath& objPath, const MDagPath& cameraPath) const{
	MDistance sizeVal = getPlugValue<MDistance>(objPath, RigSystemControlNode::size, 1.0);
	int enumInt = getPlugValue<int>(objPath, RigSystemControlNode::controlList, 0);
	MPlug plug(fRigSystemControl, RigSystemControlNode::offsetMatrix);
	MMatrix offsetMatrix = plug.asMDataHandle().asMatrix();
	
	std::vector<MPointArray> shapePoints;
	for ( const auto &myPair : control_shapes ) {
    	shapePoints.push_back(myPair.second);
	}
	MBoundingBox bbox =  getBoundingBox(shapePoints[enumInt], sizeVal, offsetMatrix);
	return bbox;
}


MUserData* RigSystemControlDrawOverride::prepareForDraw(const MDagPath& objPath,const MDagPath& cameraPath,const MHWRender::MFrameContext& frameContext,MUserData* oldData){
	MProfilingScope profilerScope(gProfilerCategory, MProfiler::kColorA_L1, "MPxDrawOverride::prepareForDraw()");

	RigSystemControlData* data = dynamic_cast<RigSystemControlData*>(oldData);
	if (!data)	{
		data = new RigSystemControlData();
	}

	MPlug plug(fRigSystemControl, RigSystemControlNode::offsetMatrix);
	MMatrix offsetMatrix = plug.asMDataHandle().asMatrix();
	double sizeVal = getPlugValue<double>(objPath, RigSystemControlNode::size, 1.0);
	int enumInt = getPlugValue<int>(objPath, RigSystemControlNode::controlList, 0);

	data->fDepthPriority = 5;
	data->fillObject =  getPlugValue<bool>(objPath, RigSystemControlNode::fill, false); 
	data->drawInFront = getPlugValue<bool>(objPath, RigSystemControlNode::front, false);
	data->lineThick = getPlugValue<float>(objPath, RigSystemControlNode::lineThickness, 1.0);
	data->fColor = MHWRender::MGeometryUtilities::wireframeColor(objPath);

	std::vector<MPointArray> shapePoints;
	for ( const auto &myPair : control_shapes ) {
    	shapePoints.push_back(myPair.second);
	}
	
	data->fLineList.clear();
	for (unsigned int c = 0; c < shapePoints[enumInt].length(); c++) {
		if (c%3 != 0){
			data->fLineList.append((shapePoints[enumInt][c] * sizeVal) * offsetMatrix);
		}
	}

	data->fTriangleList.clear();
	if (data->fillObject == true) {
		for (unsigned int c = 0; c < shapePoints[enumInt].length(); c++) {
			data->fTriangleList.append((shapePoints[enumInt][c] * sizeVal) * offsetMatrix);
		}
	}

	return data;
}

void RigSystemControlDrawOverride::addUIDrawables( const MDagPath& objPath, MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext, const MUserData* data){
	
	MProfilingScope profilerScope(gProfilerCategory, MProfiler::kColorD_L2, "MPxDrawOverride::addUIDrawables()");

	RigSystemControlData* pLocatorData = (RigSystemControlData*)data;
	if (!pLocatorData)
	{
		return;
	}
	
	drawManager.beginDrawable();
	if (pLocatorData->drawInFront) {
		drawManager.beginDrawInXray();
	}
	drawManager.setLineWidth(pLocatorData->lineThick);
	drawManager.setColor(pLocatorData->fColor);
	drawManager.setDepthPriority(pLocatorData->fDepthPriority);

	if (pLocatorData->fillObject ){
		drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, pLocatorData->fTriangleList);
	}

	drawManager.mesh(MHWRender::MUIDrawManager::kLines, pLocatorData->fLineList);
	if (pLocatorData->drawInFront) {
		drawManager.endDrawInXray();
	}
	drawManager.endDrawable();
}

MStatus RigSystemControlNode::initialize()
{
	MFnUnitAttribute uAttr;
	MFnEnumAttribute eAttr;
	MFnMatrixAttribute mAttr;
	MFnNumericAttribute nAttr;

	offsetMatrix = mAttr.create("offsetMatrix", "om", MFnMatrixAttribute::kDouble);
	mAttr.setKeyable(false);
	mAttr.setReadable(true);
	mAttr.setStorable(true);
	addAttribute(offsetMatrix);

	controlList = eAttr.create("controlList", "cl", 0);
	int enumindex = 0;
	std::vector<MString> keys;
	for ( const auto &myPair : control_shapes ) {
		eAttr.addField(myPair.first.c_str(), enumindex);
		enumindex++;
    }
	eAttr.setReadable(true);
	addAttribute(controlList);

	size = uAttr.create("size", "sz", MFnUnitAttribute::kDistance);
	uAttr.setDefault(1.0);
	uAttr.setSoftMin(0.0);
	uAttr.setSoftMax(10.0);
	addAttribute(size);
	
	lineThickness = nAttr.create("lineThickness", "ltn", MFnNumericData::kFloat, 1.0);
	nAttr.setSoftMin(0.0);
	nAttr.setSoftMax(10.0);
	addAttribute(lineThickness);

	fill = nAttr.create("fill", "fl", MFnNumericData::kBoolean, 0);
	nAttr.setDefault(false);
	nAttr.setWritable(true);
	nAttr.setReadable(true);
	addAttribute(fill);

	front = nAttr.create("drawInFront", "dif", MFnNumericData::kBoolean, 0);
	nAttr.setDefault(false);
	nAttr.setWritable(true);
	nAttr.setReadable(true);
	addAttribute(front);

	worldS = uAttr.create("worldS", "ws", MFnUnitAttribute::kDistance, 1.0);
	uAttr.setWritable(true);
	uAttr.setCached(false);
	uAttr.setHidden(true);
	addAttribute(worldS);

	attributeAffects(size, worldS);
	attributeAffects(controlList, worldS);
	attributeAffects(offsetMatrix, worldS);
	attributeAffects(fill, worldS);
	attributeAffects(front, worldS);
	attributeAffects(lineThickness, worldS);

	return MS::kSuccess;
}
