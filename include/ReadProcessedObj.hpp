#ifndef ReadProcessedObj_hpp
#define ReadProcessedObj_hpp

#include <mat4x4/vec3.hpp>
#include <cstdint>
#include <stdlib.h>

struct FileObjectDivisionData {
	int32_t start;
	size_t count;
	uint8_t usemtl[512];
};
struct ObjectDivisionData {
	int32_t start;
	size_t count;
	uint32_t texture;
};
struct ObjectData {
	unsigned int vertices_n;
	float *vertices;
	
	ObjectDivisionData *divisionData;
	unsigned int divisionsN;
};

template <unsigned int d, typename T> struct obj_array_struct {
	T array[d];
	T &operator[](unsigned int index) {
		return array[index];
	}
	obj_array_struct<d, T> &operator=(obj_array_struct<d, T> other){
		for(unsigned int i=0; i<d; i++) array[i] = other[i];
		return *this;
	}
};
struct obj_v : obj_array_struct<3, float> {};
struct obj_vt : obj_array_struct<2, float> {};
struct obj_vn : obj_array_struct<3, float> {};
vec3 OBJVToVector(obj_v OBJvertex);
obj_v VectorToOBJV(vec3 vec);
obj_vn VectorToOBJVN(vec3 vec);

// allocates both the `vertices` and `divisionData` arrays in the returned struct (so they need to be freed eventually)
ObjectData ReadProcessedOBJFile(const char *file, uint32_t (*mtlNameToTextureId)(const char *));

#endif /* ReadProcessedObj_hpp */
