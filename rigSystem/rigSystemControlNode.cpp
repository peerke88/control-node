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
	MObject thisNode = objPath.node();
	MPlug plug(thisNode, RigSystemControlNode::size);
	MDistance sizeVal;
	plug.getValue(sizeVal);

	MPlug plug1(thisNode, RigSystemControlNode::controlList);
	int enumInt;
	plug1.getValue(enumInt);
	MPlug plug2(thisNode, RigSystemControlNode::offsetMatrix);
	MMatrix offsetMatrix = plug2.asMDataHandle().asMatrix();
	
	std::vector<MPointArray> shapePoints;
	for ( const auto &myPair : control_shapes ) {
    	shapePoints.push_back(myPair.second);
	}
	MBoundingBox bbox =  getBoundingBox(shapePoints[enumInt], sizeVal, offsetMatrix);
	return bbox;
}


MUserData* RigSystemControlDrawOverride::prepareForDraw(const MDagPath& objPath,const MDagPath& cameraPath,const MHWRender::MFrameContext& frameContext,MUserData* oldData){
	RigSystemControlData* data = dynamic_cast<RigSystemControlData*>(oldData);
	if (!data)	{
		data = new RigSystemControlData();
	}

	float sizeVal = 1.0f;
	int enumInt = 0;
	MMatrix offsetMatrix;
	bool inFill = false;
	bool inFront = false;
	float thickness = 1.0f;

	MStatus status;
	MObject RigSystemControlNode = fRigSystemControl;
	if (status) {
		MPlug plug(RigSystemControlNode, RigSystemControlNode::size);
		plug.getValue(sizeVal);

		MPlug plug1(RigSystemControlNode, RigSystemControlNode::controlList);
		plug1.getValue(enumInt);

		MPlug plug2(RigSystemControlNode, RigSystemControlNode::offsetMatrix);
		offsetMatrix = plug2.asMDataHandle().asMatrix();

		MPlug plug3(RigSystemControlNode, RigSystemControlNode::fill);
		plug3.getValue(inFill);

		MPlug plug6(RigSystemControlNode, RigSystemControlNode::front);
		plug6.getValue(inFront);

		MPlug plug8(RigSystemControlNode, RigSystemControlNode::lineThickness);
		plug8.getValue(thickness);
	};

	data->fDepthPriority = 5;
	data->fillObject = inFill;
	data->drawInFront = inFront;
	data->lineThick = thickness;
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
	if (inFill == true) {
		for (unsigned int c = 0; c < shapePoints[enumInt].length(); c++) {
			data->fTriangleList.append((shapePoints[enumInt][c] * sizeVal) * offsetMatrix);
		}
	}

	return data;
}

void RigSystemControlDrawOverride::addUIDrawables( const MDagPath& objPath, MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext, const MUserData* data)
{
	RigSystemControlData* pLocatorData = (RigSystemControlData*)data;
	if (!pLocatorData)
	{
		return;
	}
	
	drawManager.beginDrawable();
	if (pLocatorData->drawInFront == true) {
		drawManager.beginDrawInXray();
	}
	drawManager.setLineWidth(pLocatorData->lineThick);
	drawManager.setColor(pLocatorData->fColor);
	drawManager.setDepthPriority(pLocatorData->fDepthPriority);

	if (pLocatorData->fillObject == true ){
		drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, pLocatorData->fTriangleList);
	}

	drawManager.mesh(MHWRender::MUIDrawManager::kLines, pLocatorData->fLineList);
	if (pLocatorData->drawInFront == true) {
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
