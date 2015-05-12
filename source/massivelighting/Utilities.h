#pragma once

class QImage;

class Utilities
{
public:
    Utilities() = delete;

	static QImage convertToGLFormat(const QImage& img);
};

