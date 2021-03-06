#include "BinaryMorphology.h"

#include "ImageScaler.h"

const uchar BinaryMorphology::DEFAULT_BACK_COLOR = 0;
const uchar BinaryMorphology::DEFAULT_FORE_COLOR = 255;

BinaryMorphology::BinaryMorphology(ImageProcessor* imageProcessor,
                                   bool whiteAsForeground) :
    Morphology(imageProcessor->getBinaryImage()),
    edgeImage(0),
    reconstructImage(0),
    conditionImage(0)
{
    setForeground(whiteAsForeground);
}

BinaryMorphology::BinaryMorphology(QImage *binaryImage,
                                   bool whiteAsForeground) :
    Morphology(binaryImage),
    edgeImage(0),
    reconstructImage(0),
    conditionImage(0)
{
    setForeground(whiteAsForeground);
}

BinaryMorphology::~BinaryMorphology()
{
    if (edgeImage) {
        delete edgeImage;
    }
    if (reconstructImage) {
        delete reconstructImage;
    }
    if (conditionImage) {
        delete conditionImage;
    }
}

void BinaryMorphology::setForeground(bool whiteAsForeground)
{
    if (whiteAsForeground) {
        foreGroundColor = 255;
        backGroundColor = 0;
    } else {
        foreGroundColor = 0;
        backGroundColor = 255;
    }
}

QImage* BinaryMorphology::dilationHelper(const QImage& image,
                                         const StructElement& se) const
{
    int width = image.width();
    int height = image.height();
    int size = width * height;
    int totalBits = width * height * 4;
    QImage* newImage = new QImage(image.size(), image.format());
    uchar* nBits = newImage->bits();

    // set all pixel to background
    for (int i = 0; i < size; ++i) {
        for (int rgb = 0; rgb < 3; ++rgb) {
            *nBits = backGroundColor;
            ++nBits;
        }
        ++nBits;
    }

    // dilation
    int minX = se.getMinX();
    int maxX = se.getMaxX();
    int minY = se.getMinY();
    int maxY = se.getMaxY();
    const uchar* oBits = image.constBits();
    nBits = newImage->bits();
    for (int h = 0; h < height; ++h){
        for (int w = 0; w < width; ++w) {
            // rgb of binary image is the same, compare blue only
            if (*oBits == foreGroundColor) {
                for (int x = minX; x < maxX; ++x) {
                    for (int y = minY; y < maxY; ++y) {
                        if (se.getValue(x, y) != SE_NOT_MATCH) {
                            // set rgb to be foregound
                            int index = ((h + y) * width + (w + x)) * 4;
                            if (index >= 0 && index < totalBits) {
                                for (int rgb = 0; rgb < 3; ++rgb) {
                                    *(nBits + index + rgb) = foreGroundColor;
                                }
                            }
                        }
                    }
                }
            }
            // to next pixel
            oBits += 4;
        }
    }
    return newImage;
}

QImage* BinaryMorphology::erosionHelper(const QImage& image,
                                        const StructElement& se) const
{
    int width = image.width();
    int height = image.height();
    QImage* newImage = new QImage(image.size(), image.format());

    // erosion
    int minX = se.getMinX();
    int maxX = se.getMaxX();
    int minY = se.getMinY();
    int maxY = se.getMaxY();
    int totalBits = width * height * 4;
    uchar* nBits = newImage->bits();
    const uchar* oBits = image.constBits();

    for (int h = 0; h < height; ++h){
        for (int w = 0; w < width; ++w) {
            bool isFore = true;
            for (int x = minX; isFore && x < maxX; ++x) {
                for (int y = minY; isFore && y < maxY; ++y) {
                    int index = ((h + y) * width + (w + x)) * 4;
                    // when se match but not foreground in image, not hit
                    if (index < 0 || index > totalBits ||
                            (se.getValue(x, y) != SE_NOT_MATCH &&
                                      *(oBits + index) != foreGroundColor)) {
                        isFore = false;
                    }
                }
            }
            int color = backGroundColor;
            if (isFore) {
                color = foreGroundColor;
            }
            for (int rgb = 0; rgb < 3; ++rgb) {
                *(nBits + rgb) = color;
            }
            nBits += 4;
        }
    }
    return newImage;
}

bool BinaryMorphology::isOneColor(bool& isAllFore, bool& isAllBack) const
{
    QImage* image = getOperatedImage();
    int size = image->width() * image->height();
    const uchar* bits = image->constBits();
    isAllFore = true;
    isAllBack = true;
    for (int i = 0; i < size; ++i) {
        if (*bits == backGroundColor) {
            // back
            isAllFore = false;
        } else {
            // fore
            isAllBack = false;
        }
        // next pixel
        bits += 4;
    }
    return isAllFore || isAllBack;
}

bool BinaryMorphology::isAllBack() const
{
    QImage* image = getOperatedImage();
    int size = image->width() * image->height();
    const uchar* bits = image->constBits();
    for (int i = 0; i < size; ++i) {
        if (*bits != backGroundColor) {
            return false;
        }
        // next pixel
        bits += 4;
    }
    return true;
}

bool BinaryMorphology::isAllFore() const
{
    QImage* image = getOperatedImage();
    int size = image->width() * image->height();
    const uchar* bits = image->constBits();
    for (int i = 0; i < size; ++i) {
        if (*bits != foreGroundColor) {
            return false;
        }
        // next pixel
        bits += 4;
    }
    return true;
}

uchar BinaryMorphology::getForeground() const
{
    return foreGroundColor;
}

uchar BinaryMorphology::getBackground() const
{
    return backGroundColor;
}

QImage* BinaryMorphology::getEdgeImage(const StructElement &se,
                                      EdgeType edgeType)
{
    if (edgeImage) {
        delete edgeImage;
    }

    QImage* origin = getOperatedImage();

    switch (edgeType) {
    case ET_STANDARD:
    {
        QImage* dilation = dilationHelper(*origin, se);
        QImage* erotion = erosionHelper(*origin, se);
        edgeImage = minusHelper(*dilation, *erotion);
        delete dilation;
        delete erotion;
    }
        break;

    case ET_INTERNAL:
    {
        QImage* erotion = erosionHelper(*origin, se);
        edgeImage = minusHelper(*origin, *erotion);
        delete erotion;
    }
        break;

    case ET_EXTERNAL:
    {
        QImage* dilation = dilationHelper(*origin, se);
        edgeImage = minusHelper(*dilation, *origin);
        delete dilation;
    }
        break;
    }

    return edgeImage;
}

QImage* BinaryMorphology::minusHelper(const QImage& left,
                                      const QImage& right) const
{
    if (left.width() != right.width() || left.height() != right.height()) {
        return 0;
    }
    int size = left.width() * left.height();
    const uchar* lBits = left.constBits();
    const uchar* rBits = right.constBits();
    QImage* result = new QImage(left.size(), left.format());
    uchar* bits = result->bits();
    for (int i = 0; i < size; ++i) {
        if (*lBits == foreGroundColor && *rBits == backGroundColor) {
            *bits = *(bits + 1) = *(bits + 2) = foreGroundColor;
        } else {
            *bits = *(bits + 1) = *(bits + 2) = backGroundColor;
        }
        lBits += 4;
        rBits +=4;
        bits += 4;
    }
    return result;
}

QImage* BinaryMorphology::intersectHelper(const QImage& left,
                                          const QImage& right) const
{
    if (left.width() != right.width() || left.height() != right.height()) {
        return 0;
    }
    int size = left.width() * left.height();
    const uchar* lBits = left.constBits();
    const uchar* rBits = right.constBits();
    QImage* result = new QImage(left.size(), left.format());
    uchar* bits = result->bits();
    for (int i = 0; i < size; ++i) {
        if (*lBits == foreGroundColor && *rBits == foreGroundColor) {
            *bits = *(bits + 1) = *(bits + 2) = foreGroundColor;
        } else {
            *bits = *(bits + 1) = *(bits + 2) = backGroundColor;
        }
        lBits += 4;
        rBits +=4;
        bits += 4;
    }
    return result;
}

QImage* BinaryMorphology::getReconstructImage(const StructElement &se)
{
    QImage* origin = getOperatedImage();
    int size = origin->width() * origin->height();

    if (reconstructImage) {
        delete reconstructImage;
    }
    reconstructImage = openingHelper(*origin, se);

    while (true) {
        QImage* newImage = dilationHelper(*reconstructImage, se);

        const uchar* oBits = origin->constBits();
        uchar* nBits = newImage->bits();
        for (int i = 0; i < size; ++i) {
            // intersection
            if (!*oBits) {
                // black in origin
                for (int rgb = 0; rgb < 4; ++rgb) {
                    *(nBits + rgb) = 0;
                }
            }
            oBits += 4;
            nBits += 4;
        }

        // check if changed from last loop
        bool changed = false;
        nBits = newImage->bits();
        const uchar* lBits = reconstructImage->constBits();
        for (int i = 0; i < size; ++i) {
            if (*nBits != *lBits) {
                changed = true;
                break;
            }
            nBits += 4;
            lBits += 4;
        }
        if (!changed) {
            break;
        }

        // delete last image
        QImage* tmp = reconstructImage;
        reconstructImage = newImage;
        delete tmp;
    }
    return reconstructImage;
}

QImage* BinaryMorphology::getConditionDilation(const StructElement& se,
                                               const QImage& mask)
{
    if (conditionImage) {
        delete conditionImage;
    }

    QImage* last = getOperatedImage();
    QImage* scaledMask;
    if (mask.width() == last->width() && mask.height() == last->height()) {
        scaledMask = new QImage(mask);
    } else {
        scaledMask = ImageScaler::getScaledImage(
                    &mask, last->width(),last->height(),
                    ImagePolicy::SP_NEAREST);
    }
    bool isFirst = true;
    while (true) {
        // dilation
        QImage* dilation = dilationHelper(*last, se);

        // intersection
        QImage* intersect = intersectHelper(*dilation, *scaledMask);
        delete dilation;

        if (sameImage(*last, *intersect)) {
            delete intersect;
            break;
        }
        if (!isFirst) {
            // not to delete origin image
            delete last;
        }
        isFirst = false;
        last = intersect;
    }
    delete scaledMask;
    conditionImage = last;
    return conditionImage;
}
