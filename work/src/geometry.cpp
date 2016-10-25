//---------------------------------------------------------------------------
//
// Copyright (c) 2015 Taehyun Rhee, Joshua Scott, Ben Allen
//
// This software is provided 'as-is' for assignment of COMP308 in ECS,
// Victoria University of Wellington, without any express or implied warranty. 
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

#include <cmath>
#include <iostream> // input/output streams
#include <fstream>  // file streams
#include <sstream>  // string streams
#include <string>
#include <stdexcept>
#include <vector>
#include "comp308.hpp"
#include "geometry.hpp"
#include "imageLoader.hpp"
#include "shaderLoader.hpp"

using namespace std;
using namespace comp308;

// textured object constructor
Geometry::Geometry(string filename, string tex) {
	isTexture = true;
	g_texture = 0;
	initTexture(tex);

	ambient[0] = 0.5f;
	ambient[1] = 0.5f;
	ambient[2] = 0.5f;
	ambient[3] = 1.0f;

	diffuse[0] = 0.5f;
	diffuse[1] = 0.5f;
	diffuse[2] = 0.5f;
	diffuse[3] = 1.0f;

	specular[0] = 0.7f;
	specular[1] = 0.7f;
	specular[2] = 0.7f;
	specular[3] = 1.0f;

	shininess = 0.78125;
	m_filename = filename;
	readOBJ(filename);
	if (m_triangles.size() > 0) {
		createDisplayListPoly();
	}
}

// material object constructor
Geometry::Geometry(string filename, string tex, float ar, float ag, float ab, float dr, float dg, float db, float sr, float sg, float sb, float shine) {
	if (tex != "noTexture"){
		isTexture = true;
		g_texture = 0;
		initTexture(tex);
}
	else isTexture = false;
	//set material properties
	ambient[0] = ar;
	ambient[1] = ag;
	ambient[2] = ab;
	ambient[3] = 1.0f;

	diffuse[0] = dr;
	diffuse[1] = dg;
	diffuse[2] = db;	
	diffuse[3] = 1.0f;

	specular[0] = sr;
	specular[1] = sg;
	specular[2] = sb;
	specular[3] = 1.0f;

	shininess = shine;

	m_filename = filename;
	readOBJ(filename);
	if (m_triangles.size() > 0) {
		createDisplayListPoly();
	}
}

void Geometry::readOBJ(string filename) {
	// Make sure our geometry information is cleared
	m_points.clear();
	m_uvs.clear();
	m_normals.clear();
	m_triangles.clear();

	// Load dummy points because OBJ indexing starts at 1 not 0
	m_points.push_back(vec3(0,0,0));
	m_uvs.push_back(vec2(0,0));
	m_normals.push_back(vec3(0,0,1));

	ifstream objFile(filename);

	if(!objFile.is_open()) {
		cerr << "Error reading " << filename << endl;
		throw runtime_error("Error :: could not open file.");
	}
	cout << "Reading file " << filename << endl;

	// good() means that failbit, badbit and eofbit are all not set
	while (objFile.good()) {
		// Pull out line from file
		string line;
		std::getline(objFile, line);
		istringstream objLine(line);
		// Pull out mode from line
		string mode;
		objLine >> mode;
		// Reading like this means whitespace at the start of the line is fine
		// attempting to read from an empty string/line will set the failbit
		if (!objLine.fail()) {
			if (mode == "v") {
				vec3 v;
				objLine >> v.x >> v.y >> v.z;
				m_points.push_back(v);
			}
			else if (mode == "vn") {
				vec3 vn;
				objLine >> vn.x >> vn.y >> vn.z;
				m_normals.push_back(vn);
			}
			else if (mode == "vt") {
				vec2 vt;
				objLine >> vt.x >> vt.y;
				m_uvs.push_back(vt);
			}
			else if (mode == "f") {
				vector<vertex> verts;
				while (objLine.good()) {
					vertex v;
					if (m_normals.size() <= 1) { // dragon format
						objLine >> v.p;		// Scan in position index
						verts.push_back(v);
					}
					else if (m_uvs.size() <= 1) { // no vt in .obj format - bunny
						objLine >> v.p;		// Scan in position index
						objLine.ignore(1);	// Ignore the '/' character
						objLine.ignore(1);	// Ignore the '/' character
						objLine >> v.n;		// Scan in normal index
						verts.push_back(v);
					}
					else { // v/vt/vn format
						objLine >> v.p;		// Scan in position index
						objLine.ignore(1);	// Ignore the '/' character
						objLine >> v.t;		// Scan in uv (texture coord) index
						objLine.ignore(1);	// Ignore the '/' character
						objLine >> v.n;		// Scan in normal index
						verts.push_back(v);
					}
				}
				// IFF we have 3 verticies, construct a triangle
				if (verts.size() == 3) {
					triangle tri;
					tri.v[0] = verts[0];
					tri.v[1] = verts[1];
					tri.v[2] = verts[2];
					m_triangles.push_back(tri);
				}
			}
		}
	}
	// If we didn't have any normals, create them
	if (m_normals.size() <= 1) createNormals();
	
	cout << "Reading OBJ file is DONE." << endl;
	cout << m_points.size()-1 << " points" << endl;
	cout << m_uvs.size()-1 << " uv coords" << endl;
	cout << m_normals.size()-1 << " normals" << endl;
	cout << m_triangles.size() << " faces" << endl;
}

//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to populate the normals for 
// the model currently loaded. Compute per face normals
// first and get that working before moving onto calculating
// per vertex normals.
//-------------------------------------------------------------
void Geometry::createNormals() {
	//initialise normals
	for (unsigned int i = 1; i < m_points.size(); i++){
		vec3 v(0, 0, 0);
		m_normals.push_back(v);
	}
	vec3 p1;
	vec3 p2;
	vec3 p3;
	vec3 vectorU;
	vec3 vectorV;
	vec3 uCrossV;
	vec3 normalised;

	for (unsigned int i = 0; i < m_triangles.size(); i++){
		triangle tri = m_triangles[i];
		p1 = m_points[tri.v[0].p];
		p2 = m_points[tri.v[1].p];
		p3 = m_points[tri.v[2].p];

		//set vectors p1->p2, p1->p3
		vectorU = p2 - p1;
		vectorV = p3 - p1;
		// cross product and normalise
		uCrossV = cross(vectorU, vectorV);
		normalised = normalize(uCrossV);

		// set the normal index for triangle
		for (unsigned int j = 0; j < 3; j++){
			int index = m_triangles[i].v[j].p;
			vec3 currentNormal = m_normals[index]+normalised;
			vec3 updateNormal = currentNormal + normalised;
			m_normals[index] = updateNormal;
			m_triangles[i].v[j].n = index;
		}
	}
	// normalize all normals
	for (unsigned int i = 0; i < m_normals.size(); i++){
		vec3 normalized = normalize(m_normals[i]);
		m_normals[i] = normalized;
	}
}

void Geometry::createDisplayListPoly() {
	// Delete old list if there is one
	if (m_displayListPoly) glDeleteLists(m_displayListPoly, 1);

	// Create a new list
	cout << "Creating Poly Geometry" << endl;
	m_displayListPoly = glGenLists(1);
	glNewList(m_displayListPoly, GL_COMPILE);


	// Populate list
//	glColor3f(rVal, gVal, bVal);
	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < m_triangles.size(); i++) {
		triangle tri = m_triangles[i];
		for (unsigned int j = 0; j < 3; j++) {
			// Set the normal for this vertex
			int normIndex = tri.v[j].n;
			vec3 vn = m_normals[normIndex];
			glNormal3d(vn.x, vn.y, vn.z);
			// Set the vertex
			int index = tri.v[j].p;
			vec3 vp = m_points[index];
			if (isTexture) {
				vec2 texCoord = m_uvs[tri.v[j].t];
				glTexCoord2f(texCoord.x * 5.0f, texCoord.y * 5.0f);
			}
			glVertex3d(vp.x, vp.y, vp.z);
		}
	}
	glEnd();
	glEndList();
	cout << "Finished creating Poly Geometry" << endl;
}

void Geometry::renderGeometry() {



	if (isTexture) {		
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
//		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_texture);
	}
	//else do material
	
	applyMaterial();
//	glShadeModel(GL_SMOOTH);
	glCallList(m_displayListPoly);
	if (isTexture) {
		glDisable(GL_TEXTURE_2D);
	}


}

void Geometry::initTexture(std::string fname) {
	image tex(fname);

	glActiveTexture(GL_TEXTURE0); // Use slot 0, need to use GL_TEXTURE1 ... etc if using more than one texture PER OBJECT
	glGenTextures(1, &g_texture); // Generate texture ID
	glBindTexture(GL_TEXTURE_2D, g_texture); // Bind it as a 2D texture

											 // Setup sampling strategies
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Finnaly, actually fill the data into our texture
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tex.w, tex.h, tex.glFormat(), GL_UNSIGNED_BYTE, tex.dataPointer());
	
}

void Geometry::applyMaterial(){
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess*128.0);
}
