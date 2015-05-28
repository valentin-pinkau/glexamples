#include <gloperate/plugin/plugin_api.h>

#include "massivelighting/MassiveLightingPainter.h"
#include "basiclighting/BasicLightingPainter.h"

#include <glexamples-version.h>


GLOPERATE_PLUGIN_LIBRARY

    GLOPERATE_PLUGIN(MassiveLightingPainter
    , "MassiveLighting"
    , "Deffered lighting with many lights"
    , GLEXAMPLES_AUTHOR_ORGANIZATION
    , "v0.0.1" )

	GLOPERATE_PLUGIN(BasicLightingPainter
	, "BasicLighting"
	, "Deffered lighting with many lights"
	, GLEXAMPLES_AUTHOR_ORGANIZATION
	, "v0.0.1")

GLOPERATE_PLUGIN_LIBRARY_END
