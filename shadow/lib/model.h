#include"mesh.h"
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>
class Model
{
public:
    std::vector<Mesh> meshes;
    std::map<std::string, GLuint> textureMap;
    Model(std::string filepath) {
        load(filepath);
    }
    void load(std::string filepath)
    {
        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(filepath, aiProcess_Triangulate);
        // 异常处理
        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "读取模型出现错误: " << import.GetErrorString() << std::endl;
            exit(-1);
        }
        // 模型文件相对路径
        std::string rootPath = filepath.substr(0, filepath.find_last_of('/'));

        // 循环生成 mesh
        for (int i = 0; i < scene->mNumMeshes; i++)
        {
            // 引用当前mesh
            meshes.push_back(Mesh());
            Mesh& mesh = meshes.back();

            // 获取 assimp 的读取到的 aimesh 对象
            aiMesh* aimesh = scene->mMeshes[i];

            // 我们将数据传递给我们自定义的mesh
            for (int j = 0; j < aimesh->mNumVertices; j++)
            {
                // 顶点
                glm::vec3 vvv;
                vvv.x = aimesh->mVertices[j].x;
                vvv.y = aimesh->mVertices[j].y;
                vvv.z = aimesh->mVertices[j].z;
                mesh.vertexPosition.push_back(vvv);

                // 法线
                vvv.x = aimesh->mNormals[j].x;
                vvv.y = aimesh->mNormals[j].y;
                vvv.z = aimesh->mNormals[j].z;
                mesh.vertexNormal.push_back(vvv);

                // 纹理坐标: 如果存在则加入。assimp 默认可以有多个纹理坐标 我们取第一个（0）即可
                glm::vec2 vv(0, 0);
                if (aimesh->mTextureCoords[0])
                {
                    vv.x = aimesh->mTextureCoords[0][j].x;
                    vv.y = aimesh->mTextureCoords[0][j].y;
                }
                mesh.vertexTexcoord.push_back(vv);
            }

            // 如果有材质，那么传递材质
            if (aimesh->mMaterialIndex >= 0)
            {
                // 获取当前 aimesh 的材质对象
                aiMaterial* material = scene->mMaterials[aimesh->mMaterialIndex];

                // 获取 diffuse 贴图文件路径名称 我们只取1张贴图 故填 0 即可
                aiString aistr;
                if (material->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
                    
                    GLuint tex;
                    glGenTextures(1, &tex);
                    glBindTexture(GL_TEXTURE_2D, tex);
                    GLbyte color[3] = { 125 , 125 ,125 };
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, color);
                    mesh.diffuseTexture=tex;
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
                else {
                    material->GetTexture(aiTextureType_DIFFUSE, 0, &aistr);
                    std::string texpath = aistr.C_Str();
                    texpath = rootPath + '/' + texpath;   // 取相对路径

                    // 如果没生成过纹理，那么生成它
                    if (textureMap.find(texpath) == textureMap.end())
                    {
                        // 生成纹理
                        GLuint tex;
                        glGenTextures(1, &tex);
                        glBindTexture(GL_TEXTURE_2D, tex);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                        int textureWidth, textureHeight, nrComponents;

                        unsigned char* data = stbi_load(texpath.c_str(), &textureWidth, &textureHeight, &nrComponents, 0);
                        if (data)
                        {
                            GLenum format;
                            if (nrComponents == 1)
                                format = GL_RED;
                            else if (nrComponents == 3)
                                format = GL_RGB;
                            else if (nrComponents == 4)
                                format = GL_RGBA;
                            glTexImage2D(GL_TEXTURE_2D, 0, format, textureWidth, textureHeight, 0, format, GL_UNSIGNED_BYTE, data);
                            stbi_image_free(data);
                        }
                        textureMap[texpath] = tex;
                    }

                    // 传递纹理
                    mesh.diffuseTexture = textureMap[texpath];
                }

            }

            // 传递面片索引
            for (GLuint j = 0; j < aimesh->mNumFaces; j++)
            {
                aiFace face = aimesh->mFaces[j];
                for (GLuint k = 0; k < face.mNumIndices; k++)
                {
                    mesh.index.push_back(face.mIndices[k]);
                }
            }

            mesh.bindData();
        }
    }
    void draw(Shader& shader)
    {
        for (int i = 0; i < meshes.size(); i++)
        {
            meshes[i].draw(shader);
        }
    }
};
