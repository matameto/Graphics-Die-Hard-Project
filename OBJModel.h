#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <glut.h>
#include "GLTexture.h"


class OBJModel {
public:
    struct Vertex {
        float x, y, z;
    };

    struct TexCoord {
        float u, v;
    };

    struct Normal {
        float x, y, z;
    };

    struct Face {
        std::vector<int> vertexIndices;
        std::vector<int> texCoordIndices;
        std::vector<int> normalIndices;
        std::string material; // Material name
    };
    struct Material {
        std::string name;
        float diffuse[3]; // Diffuse color (RGB)
        float ambient[3]; // Ambient color (RGB)
        float specular[3]; // Specular color (RGB)
        float shininess;   // Shininess
        GLTexture texture; // GLTexture object
    };


private:
    std::vector<Vertex> vertices;
    std::vector<TexCoord> texCoords;
    std::vector<Normal> normals;
    std::vector<Face> faces;
    std::map<std::string, Material> materials; // Materials map
    GLuint displayList;
    bool hasTexCoords;
    bool hasNormals;

public:
    OBJModel() : displayList(0), hasTexCoords(false), hasNormals(false) {}

    bool LoadFromFile(const char* filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        std::string directory = GetDirectory(filename);
        std::string currentMaterial;

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string token;
            iss >> token;

            if (token == "v") {
                Vertex vertex;
                iss >> vertex.x >> vertex.y >> vertex.z;
                vertices.push_back(vertex);
            }
            else if (token == "vt") {
                TexCoord texCoord;
                iss >> texCoord.u >> texCoord.v;
                texCoords.push_back(texCoord);
                hasTexCoords = true;
            }
            else if (token == "vn") {
                Normal normal;
                iss >> normal.x >> normal.y >> normal.z;
                normals.push_back(normal);
                hasNormals = true;
            }
            else if (token == "f") {
                Face face;
                face.material = currentMaterial;
                std::string vertex;
                while (iss >> vertex) {
                    std::istringstream vertexStream(vertex);
                    std::string indexStr;
                    int vertexIndex = -1, texCoordIndex = -1, normalIndex = -1;

                    if (std::getline(vertexStream, indexStr, '/')) {
                        vertexIndex = std::stoi(indexStr) - 1;
                    }

                    if (std::getline(vertexStream, indexStr, '/')) {
                        if (!indexStr.empty()) texCoordIndex = std::stoi(indexStr) - 1;
                    }

                    if (std::getline(vertexStream, indexStr, '/')) {
                        if (!indexStr.empty()) normalIndex = std::stoi(indexStr) - 1;
                    }

                    face.vertexIndices.push_back(vertexIndex);
                    face.texCoordIndices.push_back(texCoordIndex);
                    face.normalIndices.push_back(normalIndex);
                }
                faces.push_back(face);
            }
            else if (token == "usemtl") {
                iss >> currentMaterial;
            }
            else if (token == "mtllib") {
                std::string mtlFile;
                iss >> mtlFile;
                LoadMaterialLibrary((directory + "/" + mtlFile).c_str());
            }
        }

        file.close();
        CreateDisplayList();
        return true;
    }

    void LoadMaterialLibrary(const char* filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open material file " << filename << std::endl;
            return;
        }

        std::string directory = GetDirectory(filename);
        std::string line;
        Material material;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string token;
            iss >> token;

            if (token == "newmtl") {
                if (!material.name.empty()) {
                    materials[material.name] = material; // Save the current material
                }
                material = Material(); // Start a new material
                iss >> material.name;
            }
            else if (token == "Kd") {
                iss >> material.diffuse[0] >> material.diffuse[1] >> material.diffuse[2];
            }
            else if (token == "Ka") {
                iss >> material.ambient[0] >> material.ambient[1] >> material.ambient[2];
            }
            else if (token == "Ks") {
                iss >> material.specular[0] >> material.specular[1] >> material.specular[2];
            }
            else if (token == "Ns") {
                iss >> material.shininess;
            }
            else if (token == "map_Kd") {
                std::string textureFile;
                iss >> textureFile;
                std::string fullPath = directory + "/" + textureFile;
                material.texture.Load(const_cast<char*>(fullPath.c_str())); // Use your GLTexture loader
            }
        }

        if (!material.name.empty()) {
            materials[material.name] = material; // Save the last material
        }

        file.close();
    }



    GLuint LoadTexture(const char* filename) {
        // Dummy implementation to ensure it doesn't fail
        return 0; // Placeholder
    }

    void CreateDisplayList() {
        displayList = glGenLists(1);
        glNewList(displayList, GL_COMPILE);

        for (Face& face : faces) { // Removed const
            if (materials.find(face.material) != materials.end()) {
                Material& material = materials[face.material]; // Removed const
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material.diffuse);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material.ambient);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material.specular);
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material.shininess);

                if (material.texture.texture[0] != 0) {
                    material.texture.Use();
                }
                else {
                    glDisable(GL_TEXTURE_2D);
                }
            }

            glBegin(face.vertexIndices.size() == 3 ? GL_TRIANGLES : GL_POLYGON);
            for (size_t i = 0; i < face.vertexIndices.size(); i++) {
                if (hasTexCoords && face.texCoordIndices[i] >= 0) {
                    const TexCoord& texCoord = texCoords[face.texCoordIndices[i]];
                    glTexCoord2f(texCoord.u, texCoord.v);
                }
                if (hasNormals && face.normalIndices[i] >= 0) {
                    const Normal& normal = normals[face.normalIndices[i]];
                    glNormal3f(normal.x, normal.y, normal.z);
                }
                const Vertex& vertex = vertices[face.vertexIndices[i]];
                glVertex3f(vertex.x, vertex.y, vertex.z);
            }
            glEnd();
        }

        glEndList();
    }



    void Draw() const {
        if (displayList != 0) {
            glCallList(displayList);
        }
    }

    ~OBJModel() {
        if (displayList != 0) {
            glDeleteLists(displayList, 1);
        }
    }

private:
    std::string GetDirectory(const std::string& filepath) const {
        size_t pos = filepath.find_last_of("/\\");
        return (pos == std::string::npos) ? "" : filepath.substr(0, pos);
    }
};

#endif // OBJ_LOADER_H
