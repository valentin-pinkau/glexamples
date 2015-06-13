#pragma once


#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>






class MassiveLightingClusterStage : public gloperate::AbstractStage
{
public:
    MassiveLightingClusterStage();
    virtual ~MassiveLightingClusterStage() = default;
    virtual void initialize() override;

public:


protected:
    virtual void process() override;



protected:

};


