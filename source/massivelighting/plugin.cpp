#include <gloperate/plugin/plugin_api.h>

#include "massivelighting/MassiveLightingPainter.h"
#include "basiclighting/BasicLightingPainter.h"

#include <glexamples-version.h>


GLOPERATE_PLUGIN_LIBRARY

	GLOPERATE_PAINTER_PLUGIN(MassiveLightingPainter
    , "MassiveLighting"
    , "Deferred lighting with many lights"
    , GLEXAMPLES_AUTHOR_ORGANIZATION
    , "v0.0.1" )

	GLOPERATE_PAINTER_PLUGIN(BasicLightingPainter
	, "BasicLighting"
	, "Basic lighting with few lights"
	, GLEXAMPLES_AUTHOR_ORGANIZATION
	, "v0.0.1")

GLOPERATE_PLUGIN_LIBRARY_END
