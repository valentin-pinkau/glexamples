#pragma once

#include <memory>
#include <vector>

#include "assimp/PolygonalDrawable.h"
#include "assimp/PolygonalGeometry.h"

#include <glm/vec3.hpp>

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>

#include <reflectionzeug/FilePath.h>


class GeometryStage : public gloperate::AbstractStage
{
public:
    GeometryStage();
    virtual ~GeometryStage() = default;
    virtual void initialize() override;

public:
    gloperate::InputSlot<reflectionzeug::FilePath> sceneFilePath;
    gloperate::Data<std::vector<std::unique_ptr<PolygonalDrawable>>> drawables;

protected:
    virtual void process() override;
    void reloadScene();

};


