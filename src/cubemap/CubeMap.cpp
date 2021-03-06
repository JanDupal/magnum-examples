/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include "CubeMap.h"

#include <fstream>
#include <Utility/Resource.h>
#include <CubeMapTexture.h>
#include <MeshTools/FlipNormals.h>
#include <MeshTools/Interleave.h>
#include <MeshTools/CompressIndices.h>
#include <Primitives/Cube.h>
#include <SceneGraph/Scene.h>
#include <SceneGraph/Camera3D.h>
#include <Trade/AbstractImporter.h>
#include <Trade/ImageData.h>
#include <Trade/MeshData3D.h>

#include "CubeMapShader.h"

using namespace Corrade::Utility;

namespace Magnum { namespace Examples {

CubeMap::CubeMap(const std::string& prefix, Object3D* parent, SceneGraph::DrawableGroup3D<>* group): Object3D(parent), SceneGraph::Drawable3D<>(this, group) {
    CubeMapResourceManager* resourceManager = CubeMapResourceManager::instance();

    /* Cube mesh */
    if(!(cube = resourceManager->get<Mesh>("cube"))) {
        Mesh* mesh = new Mesh;
        Buffer* buffer = new Buffer;
        Buffer* indexBuffer = new Buffer;

        Trade::MeshData3D cubeData = Primitives::Cube::solid();
        MeshTools::flipFaceWinding(*cubeData.indices());
        MeshTools::compressIndices(mesh, indexBuffer, Buffer::Usage::StaticDraw, *cubeData.indices());
        MeshTools::interleave(mesh, buffer, Buffer::Usage::StaticDraw, *cubeData.positions(0));
        mesh->setPrimitive(cubeData.primitive())
            ->addVertexBuffer(buffer, 0, CubeMapShader::Position());

        resourceManager->set("cube-buffer", buffer, ResourceDataState::Final, ResourcePolicy::Resident);
        resourceManager->set("cube-index-buffer", indexBuffer, ResourceDataState::Final, ResourcePolicy::Resident);
        resourceManager->set(cube.key(), mesh, ResourceDataState::Final, ResourcePolicy::Resident);
    }

    /* Cube map texture */
    if(!(texture = resourceManager->get<CubeMapTexture>("texture"))) {
        CubeMapTexture* cubeMap = new CubeMapTexture;

        cubeMap->setWrapping(CubeMapTexture::Wrapping::ClampToEdge)
            ->setMagnificationFilter(CubeMapTexture::Filter::Linear)
            ->setMinificationFilter(CubeMapTexture::Filter::Linear, CubeMapTexture::Mipmap::Linear);

        Resource<Trade::AbstractImporter> importer = resourceManager->get<Trade::AbstractImporter>("tga-importer");

        /* Configure texture storage using size of first image */
        importer->openFile(prefix + "+x.tga");
        Trade::ImageData2D* image = importer->image2D(0);
        Vector2i size = image->size();
        cubeMap->setStorage(Math::log2(size.min())+1, CubeMapTexture::InternalFormat::RGB8, size);
        cubeMap->setSubImage(CubeMapTexture::PositiveX, 0, {}, image);
        delete image;

        importer->openFile(prefix + "-x.tga");
        image = importer->image2D(0);
        cubeMap->setSubImage(CubeMapTexture::NegativeX, 0, {}, image);
        delete image;

        importer->openFile(prefix + "+y.tga");
        image = importer->image2D(0);
        cubeMap->setSubImage(CubeMapTexture::PositiveY, 0, {}, image);
        delete image;

        importer->openFile(prefix + "-y.tga");
        image = importer->image2D(0);
        cubeMap->setSubImage(CubeMapTexture::NegativeY, 0, {}, image);
        delete image;

        importer->openFile(prefix + "+z.tga");
        image = importer->image2D(0);
        cubeMap->setSubImage(CubeMapTexture::PositiveZ, 0, {}, image);
        delete image;

        importer->openFile(prefix + "-z.tga");
        image = importer->image2D(0);
        cubeMap->setSubImage(CubeMapTexture::NegativeZ, 0, {}, image);
        delete image;

        cubeMap->generateMipmap();

        resourceManager->set(texture.key(), cubeMap, ResourceDataState::Final, ResourcePolicy::Manual);
    }

    /* Shader */
    if(!(shader = resourceManager->get<AbstractShaderProgram, CubeMapShader>("shader")))
        resourceManager->set<AbstractShaderProgram>(shader.key(), new CubeMapShader, ResourceDataState::Final, ResourcePolicy::Manual);
}

void CubeMap::draw(const Matrix4& transformationMatrix, SceneGraph::AbstractCamera3D<>* camera) {
    shader->setTransformationProjectionMatrix(camera->projectionMatrix()*transformationMatrix)
        ->use();

    texture->bind(CubeMapShader::TextureLayer);

    cube->draw();
}

}}
