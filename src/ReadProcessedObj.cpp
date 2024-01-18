#include "ReadProcessedObj.hpp"

#include <stdio.h>
#include <iostream>

#define MAX_CORNERS 128
struct obj_corner {
	uint32_t vIndex, vtIndex, vnIndex;
};
struct obj_smoothVertex {
	obj_v v;
	obj_vn vn;
	obj_vt vt;
};
vec3 OBJVToVector(obj_v OBJvertex){
	return {OBJvertex[0], OBJvertex[1], OBJvertex[2]};
}
obj_v VectorToOBJV(vec3 vec){
	obj_v ret;
	ret[0] = vec.x;
	ret[1] = vec.y;
	ret[2] = vec.z;
	return ret;
}
obj_vn VectorToOBJVN(vec3 vec){
	obj_vn ret;
	ret[0] = vec.x;
	ret[1] = vec.y;
	ret[2] = vec.z;
	return ret;
}

// allocates both the `vertices` and `divisionData` arrays in the returned struct (so they need to be freed eventually)
ObjectData ReadProcessedOBJFile(const char *file, uint32_t (*mtlNameToTextureId)(const char *)){
	ObjectData ret;
	
	FILE *fptr;
	fptr = fopen(file, "rb");
	if(!fptr){ std::cout << "ERROR: Unable to open file for reading." << std::endl; return ret; }
	fread(&ret.vertices_n, sizeof(uint32_t), 1, fptr);
	ret.vertices = (float *)malloc(ret.vertices_n * 8 * sizeof(float));
	fread(ret.vertices, sizeof(uint32_t), ret.vertices_n * 8, fptr);
	fread(&ret.divisionsN, sizeof(uint32_t), 1, fptr);
	FileObjectDivisionData *fileDivData = (FileObjectDivisionData *)malloc(ret.divisionsN * sizeof(FileObjectDivisionData));
	fread(fileDivData, sizeof(FileObjectDivisionData), ret.divisionsN, fptr);
	ret.divisionData = (ObjectDivisionData *)malloc(ret.divisionsN * sizeof(ObjectDivisionData));
	for(int i=0; i<ret.divisionsN; i++){
		ret.divisionData[i].start = fileDivData[i].start;
		ret.divisionData[i].count = (size_t)fileDivData[i].count;
		ret.divisionData[i].texture = mtlNameToTextureId((const char *)fileDivData[i].usemtl);
	}
	free(fileDivData);
	fclose(fptr);
	return ret;
}
