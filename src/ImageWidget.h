#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QPixmap>
#include <QWidget>

#include "ImageLabel.h"
#include "MainWindow.h"

namespace Ui {
    class ImageWidget;
}

class ImageLabel;

class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWidget(MainWindow* mainWindow,
                         QImage* image, QWidget *parent = 0);
    ~ImageWidget();

    void setImage(QImage* image);
    QImage* getImage();

    // called by ImageLabel to update rgb info
    void setMousePosition(int x, int y);

private slots:
    void on_horizontalSlider_valueChanged(int value);

    void on_saveButton_clicked();

    void on_applyButton_clicked();

private:
    static const int PIXEL_SIZE =4;
    Ui::ImageWidget *ui;
    ImageLabel* imageLabel;
    QImage* image;

    MainWindow* mainWindow;
};

#endif // IMAGEWIDGET_H
