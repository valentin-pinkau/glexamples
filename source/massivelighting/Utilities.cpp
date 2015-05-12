#include "Utilities.h"

#include <QGLWidget>

// The gl-header included by Qt is not compatible with glbinding
// Therefore call the Qt-conversion function in a separate cpp
QImage Utilities::convertToGLFormat(const QImage& img)
{
	return QGLWidget::convertToGLFormat(img);
}