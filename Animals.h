#pragma once
#include <vector>
#include "Vector3.h"
#include "Vector4.h"
#include "Camera.h"

using namespace std;
class Animals
{
protected:
	vector<Vector3> bun, drag, bunv, dragv;
	// ref_c = unpie * mat_c * [(L_pos - Obj_pos) dot norm]
	vector<Vector4> bun4, drag4;
	Vector3 smallB, largeB, smallD, largeD;

public:
	//vector<double> bunc, dragc;

	Animals();
	~Animals();

	vector<Vector3> getBunny();
	vector<Vector3> getBunnyNorm();
	vector<Vector4> getBunny4();

	vector<Vector3> getDragon();
	vector<Vector3> getDragonNorm();
	vector<Vector4> getDragon4();

	Vector3 getSmallBunny();
	Vector3 getLargeBunny();
	Vector3 getSmallDragon();
	Vector3 getLargeDragon();

	void quickParse();
	void parseBunny();
	void parseDragon();
};

